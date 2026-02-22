## 5. Installation & Deployment
Server Setup (Linux)
1- Initialize DB: "docker-compose up -d"
2- Generate SSL: openssl req ... -out server.crt
3- Run Server: "python manage.py runserver_plus 0.0.0.0:443 --cert-file server.crt --key-file server.key"

Client Compilation (Linux Cross-Compile) :

```
x86_64-w64-mingw32-g++ agent.cpp -o MorphinAgent.exe \
    -I./include -L./libs -lssl -lcrypto -lwinhttp -liphlpapi -lws2_32 \
    -ladvapi32 -lcrypt32 -lgdi32 -static -O3 -s
```