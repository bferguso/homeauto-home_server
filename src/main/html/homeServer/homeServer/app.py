from flask import request
from flask import Flask, render_template
from flask import Response
from homeServer.HomeServerDao import HomeServerDao
import simplejson as json
import datetime
import decimal
app = Flask(__name__)
app.debug = True
app.filename = "/var/www/html/log/environment.log"

#app.run(debug=True,host="0.0.0.0")
def verify_token(password):
    passw = 'thisisourtoken'
    if password == passw:
        return True

    return False

@app.route('/')
def showHomePage():
    dao = HomeServerDao()
    last_log = dao.getLastEnvironmentLog()
    return render_template("home.html", ip="192.168.102.124", latest_env=last_log)

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
    obj['date'] = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
#    file1 = open(app.filename, "a")
#    file1.write(json.dumps(obj))
#    file1.write("\n")
#    file1.close();
    obj['status']="success"
    resp = Response(response=json.dumps(obj),
                    status=200,
                    mimetype="application/json")
    dao = HomeServerDao()
    dao.saveEnvironmentLog(obj)

    return resp

@app.route('/hourlySummary')
def readEnv():
    dao = HomeServerDao()
    times = dao.getHourlyTimes()
    locations = dao.getLocationsInPeriod()
    last_log = dao.getLastEnvironmentLog()
    env_summary = {}
    print (locations)
    for location in locations:
        env_summary[location['sensor_location']] = dao.getEnvironmentLogSummary(location[0])
    return render_template("hourlySummary.html", latest_env=last_log, locations=locations, times=times, env_summary=env_summary)

@app.route('/dailySummary')
def dailySummary():
    dao = HomeServerDao()
    return render_template("dailySummary.html", env_summary=dao.getEnvironmentDailyLogSummary())


def convert_value(value):
    if isinstance(value, (datetime.datetime, datetime.date, datetime.time)):
        return value.isoformat()
    if isinstance(value, (decimal.Decimal)):
        return float(value)
    return value

if __name__ == "__main__":
    app.run(host='0.0.0.0',port=5000)
