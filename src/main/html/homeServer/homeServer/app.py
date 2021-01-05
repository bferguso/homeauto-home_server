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

def verify_token(password):
    passw = 'thisisourtoken'
    if password == passw:
        return True

    return False

@app.route('/')
def showHomePage():
    dao = HomeServerDao()
    last_devices = dao.getLastSeenDevices()
    last_log = []

    for device in last_devices:
        new_device = {}
        reading = dao.getLastEnvironmentLog(device['sensor_location'])
        new_device['sensor_location'] = device['sensor_location']
        new_device['remote_address'] = device['remote_address']
        new_device['last_seen_timestamp'] = device['last_seen_timestamp']
        new_device['device_active'] = device['device_active']
        new_device['last_reading'] = reading
        last_log.append(new_device)
    return render_template("home.html", latest_env=last_log)

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
    obj['remote_address'] = request.remote_addr
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
    env_summary = {}
    for location in locations:
        location_summary = dao.getEnvironmentLogSummary(location[0])
        location_summary.append(dao.getLastEnvironmentLog(location['sensor_location']))
        env_summary[location['sensor_location']] = location_summary
    return render_template("hourlySummary.html", locations=locations, times=times, env_summary=env_summary)

@app.route('/dailySummary')
def dailySummary():
    dao = HomeServerDao()
    times = dao.getDailyTimes()
    locations = dao.getLocationsInPeriod()
    env_summary = {}
    for location in locations:
        env_summary[location['sensor_location']] = dao.getEnvironmentDailyLogSummary(location[0])
    return render_template("dailySummary.html", locations=locations, times=times, env_summary=env_summary)

def convert_value(value):
    if isinstance(value, (datetime.datetime, datetime.date, datetime.time)):
        return value.isoformat()
    if isinstance(value, (decimal.Decimal)):
        return float(value)
    return value

if __name__ == "__main__":
    app.run(host='0.0.0.0',port=5000)
