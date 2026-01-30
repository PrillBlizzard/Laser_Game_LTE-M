# file to exec to lauch the server

from flask import Flask
from flask import request

import time
import os
import shutil

app = Flask(__name__)

@app.route("/")
def hello() -> str:
        print(" Someone said Hello World")
        return 'a Messy web page (noramlly)'

@app.post('/posty')
def adding_small_file():
        print(" Someone want to say :")
        payload  = request.data
        print(payload)
        file = open("Posty_POST_file","w")
        file.write("écriture dans le fichier")
        return "an acknowledgment for sure "




if __name__ == "__main__":
    app.run(host='0.0.0.0', port='8000')




