from django.db import models

class MorphinAgent(models.Model):
    # Auto-migration handled via views.py
    agent_id = models.CharField(max_length=64, primary_key=True)
    shared_secret = models.BinaryField(null=True)
    
    # System Discovery Info
    hostname = models.CharField(max_length=255, blank=True)
    username = models.CharField(max_length=255, blank=True)
    is_admin = models.BooleanField(default=False)
    domain = models.CharField(max_length=255, blank=True)
    os_version = models.CharField(max_length=255, blank=True)
    architecture = models.CharField(max_length=10, blank=True)
    process_id = models.IntegerField(null=True)
    ip_addresses = models.TextField(blank=True)  # Stored as JSON string
    
    # Operational State
    last_seen = models.DateTimeField(auto_now=True)
    jitter_base = models.IntegerField(default=40)
    is_active = models.BooleanField(default=True)

    def __str__(self):
        return f"{self.agent_id} - {self.hostname}"

class CommandQueue(models.Model):
    agent = models.ForeignKey(MorphinAgent, on_delete=models.CASCADE)
    command_text = models.CharField(max_length=255) # e.g., "update_jitter"
    command_value = models.IntegerField()
    is_sent = models.BooleanField(default=False)
    created_at = models.DateTimeField(auto_now_add=True)