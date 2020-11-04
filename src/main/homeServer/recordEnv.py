from mod_python import apache
import urllib.parse
import os
import json
from datetime import datetime
filename = "/var/www/html/log/environment.log"

def logEnv(req, envJson):

    obj = json.loads(envJson)
    obj['date'] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    file1 = open(filename, "a")
    file1.write(json.dumps(obj))
    file1.write("\n")
    file1.close();
    obj['status']="success"
    req.write(json.dumps(obj))

def readEnv(req):
    with open(filename, 'rb') as f:
        f.seek(-2, os.SEEK_END)
        while f.read(1) != b'\n':
            f.seek(-2, os.SEEK_CUR)
        last_line = f.readline().decode()
        req.write(last_line);