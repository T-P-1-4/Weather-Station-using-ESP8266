# ws_server.py
import asyncio
import socket
import websockets
import requests


CLOUD_TOKEN = ""
CLOUD_URL = ""
WEBSOCKET_PORT = ""  # free port from config.env, last entry
HOST = "0.0.0.0"  # use all network interfaces

#read config.env
config_file = open("../data/config.env","r").read().split("\n")
for entity in config_file:
    if entity.startswith("CLOUD_TOKEN"):
        CLOUD_TOKEN = entity.split("=")[-1]
    if entity.startswith("CLOUD_URL"):
        CLOUD_URL = entity.split("=")[-1]
    if entity.startswith("WEBSOCKET_PORT"):
        WEBSOCKET_PORT = entity.split("=")[-1]




# cloud upload
def create_file(filename, file_content):
    remote_file_path = filename + ".csv"

    url = f"{CLOUD_URL+"/ESP_DATA/"}{remote_file_path}"
    headers = {
        "Authorization": f"Bearer {CLOUD_TOKEN}"
    }

    # PUT-Request
    response = requests.put(url, data=file_content.encode('utf-8'), headers=headers)

    if response.status_code in (200, 201, 204):
        print("Datei erfolgreich erstellt!")
    else:
        print(f"Fehler beim Erstellen der Datei: {response.status_code}")
        print(response.text)


connected_clients = set()
#websocket connection
async def handler(websocket):
    print(f"Neue Verbindung von {websocket.remote_address}")
    connected_clients.add(websocket)
    try:
        async for message in websocket:
            print("Empfangen:", message)
            #debug print and connection test
            for client in connected_clients.copy():
                try:
                    await client.send(f"{websocket.remote_address}: {message}")
                except Exception:
                    connected_clients.discard(client)

            
    except websockets.ConnectionClosed:
        print("Verbindung geschlossen.")

    finally:
        if websocket in connected_clients:
            connected_clients.remove(websocket)

def get_lan_ip():
   #get local ip
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
    except Exception:
        ip = "127.0.0.1" #default localhost
    finally:
        s.close()
    return ip

async def main():
    lan_ip = get_lan_ip()
    print(f"Starte WebSocket-Server auf {HOST}:{WEBSOCKET_PORT} (erreichbar im LAN z.B. unter ws://{lan_ip}:{WEBSOCKET_PORT})")
    async with websockets.serve(handler, HOST, WEBSOCKET_PORT):
        await asyncio.Future()  #run until shutdown

if __name__ == "__main__":
    asyncio.run(main())
