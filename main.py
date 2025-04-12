import socket

def honeypot(host='0.0.0.0', port=8080):
    # Soket oluştur
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(5)
    print(f"Honeypot başlatıldı. Dinleniyor: {host}:{port}")

    try:
        while True:
            client_socket, client_address = server_socket.accept()
            print(f"Bağlantı tespit edildi: {client_address}")
            
            # Honeypot mesajı gönder
            client_socket.send(b"Bu bir honeypot sistemidir. Tüm aktiviteler kaydedilmektedir.\n")
            data = client_socket.recv(1024)
            
            print(f"Gelen veri: {data.decode('utf-8').strip()}")
            client_socket.close()
    except KeyboardInterrupt:
        print("Honeypot durduruldu.")
    finally:
        server_socket.close()

if __name__ == "__main__":
    honeypot()