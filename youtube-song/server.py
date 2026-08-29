
from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import os

HOST = "127.0.0.1"
PORT = 8765

STATE_FILE = "/home/yuji/.local/share/youtube-song/state.json"

state = {
    "active": False,
    "playing": False,
    "music": False,
    "title": "",
    "channel": "",
    "url": ""
}


def save_state():
    directory = os.path.dirname(STATE_FILE)

    os.makedirs(directory, exist_ok=True)

    temp_file = STATE_FILE + ".tmp"

    with open(temp_file, "w", encoding="utf-8") as file:
        json.dump(
            state,
            file,
            ensure_ascii=False,
            indent=2
        )

        file.flush()
        os.fsync(file.fileno())

    os.replace(
        temp_file,
        STATE_FILE
    )


class Handler(BaseHTTPRequestHandler):

    def send_json(self, data):
        body = json.dumps(
            data,
            ensure_ascii=False
        ).encode("utf-8")

        self.send_response(200)

        self.send_header(
            "Content-Type",
            "application/json; charset=utf-8"
        )

        self.send_header(
            "Content-Length",
            str(len(body))
        )

        self.send_header(
            "Access-Control-Allow-Origin",
            "*"
        )

        self.send_header(
            "Access-Control-Allow-Methods",
            "GET, POST, DELETE, OPTIONS"
        )

        self.send_header(
            "Access-Control-Allow-Headers",
            "Content-Type"
        )

        self.end_headers()

        self.wfile.write(body)


    def do_OPTIONS(self):
        self.send_response(204)

        self.send_header(
            "Access-Control-Allow-Origin",
            "*"
        )

        self.send_header(
            "Access-Control-Allow-Methods",
            "GET, POST, DELETE, OPTIONS"
        )

        self.send_header(
            "Access-Control-Allow-Headers",
            "Content-Type"
        )

        self.end_headers()


    def read_json(self):
        try:
            length = int(
                self.headers.get(
                    "Content-Length",
                    0
                )
            )

            if length <= 0:
                return {}

            raw = self.rfile.read(length)

            return json.loads(
                raw.decode("utf-8")
            )

        except Exception:
            return {}


    def do_POST(self):

        global state

        try:
            data = self.read_json()


            if self.path == "/song":

                state["title"] = str(
                    data.get(
                        "title",
                        ""
                    )
                )

                state["channel"] = str(
                    data.get(
                        "channel",
                        ""
                    )
                )

                state["url"] = str(
                    data.get(
                        "url",
                        ""
                    )
                )

                state["music"] = bool(
                    data.get(
                        "music",
                        False
                    )
                )

                state["active"] = (
                    state["music"] and
                    state["playing"]
                )

                save_state()

                print()
                print("========== MUSIC ==========")
                print("Title   :", state["title"])
                print("Channel :", state["channel"])
                print("Music   :", state["music"])
                print("Playing :", state["playing"])
                print("Active  :", state["active"])
                print("===========================")
                print()

                self.send_json({
                    "ok": True,
                    "music": state["music"],
                    "playing": state["playing"],
                    "active": state["active"],
                    "title": state["title"],
                    "channel": state["channel"]
                })

                return


            if self.path == "/playing":

                state["playing"] = bool(
                    data.get(
                        "playing",
                        False
                    )
                )

                state["active"] = (
                    state["music"] and
                    state["playing"]
                )

                save_state()

                print(
                    "Playing:",
                    state["playing"],
                    "| Active:",
                    state["active"]
                )

                self.send_json({
                    "ok": True,
                    "playing": state["playing"],
                    "music": state["music"],
                    "active": state["active"]
                })

                return


            self.send_json({
                "ok": False,
                "error": "unknown endpoint"
            })


        except Exception as error:

            print(
                "SERVER ERROR:",
                error
            )

            self.send_json({
                "ok": False,
                "error": str(error)
            })


    def do_GET(self):

        if self.path == "/state":

            self.send_json(state)

            return


        if self.path == "/control":

            self.send_json({
                "action": None
            })

            return


        self.send_json({
            "ok": False,
            "error": "not found"
        })


    def do_DELETE(self):

        global state

        if self.path == "/song":

            state = {
                "active": False,
                "playing": False,
                "music": False,
                "title": "",
                "channel": "",
                "url": ""
            }

            save_state()

            print("Music cleared")

            self.send_json({
                "ok": True
            })

            return


        self.send_json({
            "ok": False,
            "error": "unknown endpoint"
        })


    def log_message(self, format, *args):

        print(
            "[HTTP]",
            format % args
        )


if __name__ == "__main__":

    print(
        "Music server running on "
        f"http://{HOST}:{PORT}"
    )

    save_state()

    server = HTTPServer(
        (HOST, PORT),
        Handler
    )

    try:
        server.serve_forever()

    except KeyboardInterrupt:

        print(
            "\nMusic server stopped."
        )

        server.server_close()

