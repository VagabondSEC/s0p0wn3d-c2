# Inside your main project's urls.py (next to settings.py)

from django.contrib import admin
from django.urls import path, include

urlpatterns = [
    path('admin/', admin.site.urls),
    
    # --- ADD THIS LINE ---
    # This forwards all traffic to your app's urls.py
    path('', include('c2_app.urls')), 
]