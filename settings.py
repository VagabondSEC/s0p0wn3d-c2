import os

# COMMON SETTINGS
INSTALLED_APPS = [
    'django.contrib.admin',
    'django.contrib.auth',
    'django.contrib.contenttypes',
    'django.contrib.sessions',
    'django.contrib.messages',
    'django.contrib.staticfiles',
    
    # --- ADD THESE TWO LINES ---
    'sslserver',  # Required for C++ WINHTTP_FLAG_SECURE
    'c2_app',     # Your app containing the models and views
]

# DATABASE CONFIGURATION
DATABASES = {
    'default': {
        'ENGINE': 'django.db.backends.mysql',
        'NAME': 'morphin_c2',
        'USER': 'morphin_admin',
        'PASSWORD': 'admin_password_123',
        'HOST': '127.0.0.1',  # Point to localhost because DB is exposed via Docker ports
        'PORT': '3306',
        'OPTIONS': {
            'init_command': "SET sql_mode='STRICT_TRANS_TABLES'",
        },
    }
}