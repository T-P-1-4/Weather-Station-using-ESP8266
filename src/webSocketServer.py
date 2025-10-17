# ws_server.py
import asyncio
import socket
import websockets
import requests
import json
import pandas as pd
import datetime
from zeroconf.asyncio import AsyncZeroconf, AsyncServiceInfo
from io import StringIO


CLOUD_TOKEN = ""
CLOUD_URL = ""
WEBSOCKET_PORT = "" 
full_df = pd.DataFrame()

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
    remote_file_path = filename

    url = f"{CLOUD_URL}/ESP_DATA/{remote_file_path}"
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
    global full_df #due to problems with random esp logout, to minimize data loss
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
            # deserialize       
            if message.startswith('{'):
                try:
                    json_massage = json.loads(message)
                    massage_df = pd.DataFrame()
                    massage_df = pd.read_csv(StringIO(json_massage["payload"]), sep=";")
                    full_df = pd.concat([full_df, massage_df], ignore_index= True)
                except Exception as e:
                    print(f'Exception beim lesen der json massage: {e}')

            elif message.startswith('END_UPLOAD'): #got end signal
                try:
                    if not full_df.empty:
                        if 'name' in full_df.columns:
                            full_df = full_df.sort_values(by='name', ascending=True, ignore_index=True)
                    print(full_df)
                    df_str = full_df.to_csv(sep=";", index=False)
                    create_file("data_"+ datetime.datetime.now().strftime("%Y%m%d_%H%M%S") + ".csv", df_str)
                except Exception as e:
                    print(e)


            
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
    #mDNS using AsyncZeroconf
    lan_ip = get_lan_ip()
    print(f"LAN-IP: {lan_ip} (WebSocket erreichbar z.B. unter ws://meinpc.local:{WEBSOCKET_PORT})")

    desc = {'path': '/'}
    info = AsyncServiceInfo(
        "_http._tcp.local.",
        "meinpc._http._tcp.local.",
        addresses=[socket.inet_aton(lan_ip)],
        port=int (WEBSOCKET_PORT),
        properties=desc,
        server="meinpc.local."
    )

    async with AsyncZeroconf() as azc:
        await azc.async_register_service(info)
        print("mDNS-Server für meinpc.local registriert")

        # WebSocket-Server starten
        async with websockets.serve(handler, "0.0.0.0", WEBSOCKET_PORT):
            print(f"WebSocket-Server läuft auf Port {WEBSOCKET_PORT}")
            await asyncio.Future()  #run until Shutdown


if __name__ == "__main__":
    asyncio.run(main())
