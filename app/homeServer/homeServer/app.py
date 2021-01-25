from flask import request
from flask import Flask, render_template
from flask import Response
import time
from homeServer.dao.HomeServerDao import HomeServerDao
from homeServer.dao.ExternalContentDao import ExternalContentDao

import simplejson as json
import datetime
import decimal
app = Flask(__name__)
app.debug = True
externalContent = ExternalContentDao()

def verify_token(password):
    passw = 'thisisourtoken'
    if password == passw:
        return True

    return False

@app.route('/')
def home():
    dao = HomeServerDao()
    last_devices = dao.getLastSeenDevices()
    home_data = {}
    devices = []

    for device in last_devices:
        new_device = {}
        reading = dao.getLastEnvironmentLog(device['node_location'])
        new_device['node_location'] = device['node_location']
        new_device['remote_address'] = device['remote_address']
        new_device['last_seen_timestamp'] = device['last_seen_timestamp']
        new_device['node_active'] = device['node_active']
        new_device['last_reading'] = reading
        devices.append(new_device)
    home_data["devices"] = devices
    return render_template("home.html", home_data=home_data)

@app.route('/wx')
def weather():
    weather_data = {"forecasts": [externalContent.getMarineForecast(externalContent.HOWE_SOUND),
                                  externalContent.getMarineForecast(externalContent.GEORGIA_SOUTH)],
                    "current_conditions": [externalContent.getMarineConditions(externalContent.PAM_ROCKS),
                                           externalContent.getMarineConditions(externalContent.POINT_ATKINSON),
                                           externalContent.getMarineConditions(externalContent.HALIBUT_BANK)],
                    "tide_data": [externalContent.get_tides(externalContent.POINT_ATKINSON, datetime.date.today()),
                                  externalContent.get_tides(externalContent.GIBSONS, datetime.date.today())]}
    return render_template("weather.html", weather_data=weather_data)

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
def log_env():
    obj = json.loads(request.args.get('envJson'))
    obj['date'] = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    obj['remote_address'] = request.remote_addr
    obj['status']="success"
    resp = Response(response=json.dumps(obj),
                    status=200,
                    mimetype="application/json")
    dao = HomeServerDao()
    dao.saveEnvironmentLog(obj)
    return resp

#def register_capabilities():
#    obj = json.loads(request.values.get('capabilities'))

@app.route('/logBufferedEnv', methods=["POST"])
def log_buffered_env():
    before = int(round(time.time() * 1000))
    dao = HomeServerDao()
    # Get this first to be as close to the sender millis as possible
    reference_time = datetime.datetime.now()
    request_data = request.get_json()

    # Number of readings persisted to the database
    saved_readings = 0

    if 'batch' in request_data and request_data['batch']:
        reference_millis = int(request_data['referenceMillis'])
        remote_address = request.remote_addr
        env_readings = request_data['envReadings']
        for reading in env_readings:
            reading['sample_timestamp'] = reference_time + datetime.timedelta(milliseconds=(int(reading['millisOffset']) - reference_millis))
            reading['remote_address'] = remote_address
            dao.saveEnvironmentLog(reading)
            saved_readings += 1

    obj = {"date": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
           "status": "success",
           "savedReadings": saved_readings}
    after = int(round(time.time() * 1000))
    print("Save of "+str(saved_readings)+" records took "+str((after-before)/1000)+" seconds.")
    return Response(response=json.dumps(obj),
                    status=200,
                    mimetype="application/json")


@app.route('/markSensorActive', methods=["POST"])
def mark_sensor_active():
    location = request.values.get('location_name')
    remote_address = request.values.get("ip_address")
    active_str = request.values.get("active")
    if not location or not remote_address or not active_str:
        print("location: "+str(location)+", remote_address: "+str(remote_address)+", active_str: "+str(active_str))
        return Response(response=json.dumps({"success": False}),
                    status=400,
                    mimetype="application/json")
    active = active_str.lower() == 'true'
    print("Active? "+str(active))
    dao = HomeServerDao()
    dao.set_node_active(location, remote_address, active)
    return Response(response=json.dumps({'success': True}),
                    status=200,
                    mimetype="application/json")


@app.route('/hourlySummary')
def hourlySummary():
    locationscsv = request.args.get('locations')
    dao = HomeServerDao()
    if locationscsv:
        locations = map(lambda a: {'node_location': a.strip().decode('ascii')}, locationscsv.encode('ascii', 'ignore').split(b','))
    else:
        locations = dao.getLocationsInPeriod()
    times = dao.getHourlyTimes()
    env_summary = {}
    for location in locations:
        location_summary = dao.getEnvironmentLogSummary(location['node_location'])
        location_summary.append(dao.getLastEnvironmentLog(location['node_location']))
        env_summary[location['node_location']] = location_summary
    return render_template("hourlySummary.html", locations=locations, times=times, env_summary=env_summary)

@app.route('/dailySummary')
def dailySummary():
    locationscsv = request.args.get('locations')
    dao = HomeServerDao()
    times = dao.getDailyTimes()
    if locationscsv:
        locations = map(lambda a: {'node_location': a.strip().decode('ascii')}, locationscsv.encode('ascii','ignore').split(b','))
    else:
        locations = dao.getLocationsInPeriod()
    env_summary = {}
    for location in locations:
        env_summary[location['node_location']] = dao.getEnvironmentDailyLogSummary(location['node_location'])
    return render_template("dailySummary.html", locations=locations, times=times, env_summary=env_summary)

def convert_value(value):
    if isinstance(value, (datetime.datetime, datetime.date, datetime.time)):
        return value.isoformat()
    if isinstance(value, (decimal.Decimal)):
        return float(value)
    return value

if __name__ == "__main__":
    app.run(host='0.0.0.0',port=5000)
