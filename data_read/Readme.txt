g++ -o client vc.cpp -L/usr/local/lib -L/usr/lib -lssl -lcrypto 
g++ -o server domU_acceptor_server.cpp -L/usr/local/lib -L/usr/lib -lssl -lcrypto
g++ -o controlDom control_dom.cpp -L/usr/local/lib -L/usr/lib -lssl -lcrypto 

our hmac key 
168 127 101 087 055 180 238 110 132 082 150 130 175 241 059 107 057 228 252 220 090 080 078 254 161 175 046 118 098 059 245 039


Do we use static addresses for dom0 and domU?

No. We can have dynamic addresses assigned to all the entities namely verifier, app domain and control domain. 
Verifier sends request to app domain, app domain forwards this to a preconfigured dom0 ip. 
On device restart, new dom0 ip needs to be configured into the app domain. 
The control domain identifies the verifier using its MAC address. So we can have the verifier change its IP address too. 
The control domain gets the IP address of the preconfigured MAC of verifier and sends data to that IP. 

