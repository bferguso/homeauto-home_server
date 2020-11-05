from flask import request
from flask import Flask
from flask import Response
import os
import json
from datetime import datetime
app = Flask(__name__)
app.debug = True
app.filename = "/var/www/html/log/environment.log"
#app.run(debug=True,host="0.0.0.0")
def verify_token(password):
    passw = 'thisisourtoken'
    if password == passw:
        return True

    return False

@app.route('/inventory')
def getinventory():
    if not verify_token(request.args.get('token')):
        return ('token did not matched')
    data = {"message":"Have a great day"}
    data = json.dumps(data)
    resp = Response(response=data,
                    status=200,
                    mimetype="application/json")
    return resp

@app.route('/logEnv')
def logEnv():
    obj = json.loads(request.args.get('envJson'))
    obj['date'] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    file1 = open(app.filename, "a")
    file1.write(json.dumps(obj))
    file1.write("\n")
    file1.close();
    obj['status']="success"
    resp = Response(response=json.dumps(obj),
                    status=200,
                    mimetype="application/json")
    return resp

@app.route('/readEnv')
def readEnv():
    with open(app.filename, 'rb') as f:
        f.seek(-2, os.SEEK_END)
        while f.read(1) != b'\n':
            f.seek(-2, os.SEEK_CUR)
        last_line = f.readline().decode()
        resp = Response(response=last_line,
                        status=200,
                        mimetype="application/json")
        return resp

if __name__ == "__main__":
    app.run(host='0.0.0.0',port=5000)
