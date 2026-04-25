#!/bin/bash
# deploy-aegean.sh - Deploy to all three VPS nodes

LARGE="root@74.208.190.29"
MEDIUM="root@65.38.98.105"
SMALL="root@69.48.202.32"

echo "=== Deploying to LARGE (74.208.190.29) ==="
ssh $LARGE "apt update && apt install -y gcc make wget"

# Upload WordNet and build
scp /home/main/Downloads/WNprolog-3.0/prolog/wn_s.pl $LARGE:/tmp/
scp /home/main/Downloads/WNprolog-3.0/prolog/wn_g.pl $LARGE:/tmp/
scp /home/main/Downloads/WNprolog-3.0/prolog/wn_hyp.pl $LARGE:/tmp/

ssh $LARGE << 'EOF'
    mkdir -p /opt/aegean/data /opt/aegean/bin
    mv /tmp/wn_*.pl /opt/aegean/data/
    
    cat > /opt/aegean/bin/aegean-server.c << 'INNER'
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    
    #define PORT 7373
    #define AEGEAN_BASE 0x10100
    
    const char* depth_to_geom(int d) {
        static const char* geoms[] = {"point","line","triangle","tetrahedron","5-cell","8-cell","16-cell","24-cell","120-cell","600-cell"};
        return geoms[d > 9 ? 9 : d];
    }
    
    int get_depth(const char* word) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), 
            "grep -m1 \"^s([0-9]*,[0-9]*,'%s',\" /opt/aegean/data/wn_s.pl 2>/dev/null | head -1", word);
        FILE* f = popen(cmd, "r");
        char line[512];
        int synset_id = 0;
        if (f) {
            if (fgets(line, sizeof(line), f)) sscanf(line, "s(%d,", &synset_id);
            pclose(f);
        }
        if (synset_id == 0) return 0;
        
        snprintf(cmd, sizeof(cmd), 
            "grep -c \"^hyp(%d,\" /opt/aegean/data/wn_hyp.pl 2>/dev/null", synset_id);
        f = popen(cmd, "r");
        int depth = 0;
        if (f) { fscanf(f, "%d", &depth); pclose(f); }
        return depth;
    }
    
    int main() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr.s_addr = INADDR_ANY};
        bind(sock, (struct sockaddr*)&addr, sizeof(addr));
        listen(sock, 10);
        printf("Aegean Transformer running on port %d\n", PORT);
        fflush(stdout);
        
        while (1) {
            int client = accept(sock, NULL, NULL);
            if (fork() == 0) {
                char word[256] = {0};
                read(client, word, sizeof(word) - 1);
                word[strcspn(word, "\n\r ")] = 0;
                
                int depth = get_depth(word);
                char response[1024];
                snprintf(response, sizeof(response),
                    "(FS U+10100 GS %s RS U+%04X ETX)\n# Depth: %d, Geometry: %s\n",
                    word, AEGEAN_BASE + depth, depth, depth_to_geom(depth));
                write(client, response, strlen(response));
                close(client);
                exit(0);
            }
            close(client);
        }
        return 0;
    }
INNER
    
    gcc -o /opt/aegean/bin/aegean-server /opt/aegean/bin/aegean-server.c
    nohup /opt/aegean/bin/aegean-server > /var/log/aegean.log 2>&1 &
    echo "LARGE deployed"
EOF

echo "=== Deploying to MEDIUM (65.38.98.105) ==="
scp /home/main/Documents/Tron/polyform/omicron-gnomon-engine.html $MEDIUM:/tmp/

ssh $MEDIUM << 'EOF'
    apt update && apt install -y nginx
    mv /tmp/omicron-gnomon-engine.html /var/www/html/index.html
    systemctl restart nginx
    echo "MEDIUM deployed"
EOF

echo "=== Deploying to SMALL (69.48.202.32) ==="
ssh $SMALL << 'EOF'
    apt update && apt install -y nginx
    cat > /etc/nginx/sites-available/aegean << 'INNER'
upstream transformer { server 74.208.190.29:7373; }
upstream renderer { server 65.38.98.105:80; }

server {
    listen 80;
    location /api/ {
        proxy_pass http://transformer/;
        proxy_set_header Host $host;
    }
    location / {
        proxy_pass http://renderer/;
        proxy_set_header Host $host;
    }
    add_header X-Aegean "Gnomon";
}
INNER
    ln -sf /etc/nginx/sites-available/aegean /etc/nginx/sites-enabled/
    rm -f /etc/nginx/sites-enabled/default
    systemctl restart nginx
    echo "SMALL deployed"
EOF

echo ""
echo "=== DEPLOYMENT COMPLETE ==="
echo "LARGE:  http://74.208.190.29  (transformer on :7373)"
echo "MEDIUM: http://65.38.98.105  (WebGL engine)"
echo "SMALL:  http://69.48.202.32  (proxy)"
