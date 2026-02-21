from django.urls import path, re_path
from .c2_app import views

urlpatterns = [
    # This Regex catches all 4 of your Morphin endpoints and sends them to the handler
    re_path(r'^power-rangers-about-game/.*', views.c2_heartbeat_handler),
]