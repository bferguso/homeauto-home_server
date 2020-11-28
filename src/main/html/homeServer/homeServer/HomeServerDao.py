import psycopg2
import psycopg2.extras


class HomeServerDao:
    def __init__(self):
        pass

    user = "homeauto"
    password = "homeauto"
    database = "homeauto"

    def saveEnvironmentLog(self, envData):
        conn = psycopg2.connect("dbname="+self.database+" user="+self.user+" password="+self.password)
        cur = conn.cursor()
        cur.execute("insert into ha_environment_log(sensor_location,temperature,humidity, pressure, altitude) values (%s, %s, %s, %s, %s);"
                    , (envData['location'], envData['temp'], envData['humidity'], envData['pressure'], envData['alt']))
        conn.commit()
        cur.close()
        conn.close()

    def getLastEnvironmentLog(self):
        conn = psycopg2.connect("dbname="+self.database+" user="+self.user+" password="+self.password)
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("select * from ha_environment_log order by sample_timestamp desc;");
        lastLog = cur.fetchone()
        cur.close()
        conn.close()
        return lastLog

    def getEnvironmentLogSummary(self):
        conn = psycopg2.connect("dbname="+self.database+" user="+self.user+" password="+self.password)
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
            select sensor_location,
               date_trunc('hour', sample_timestamp) sample_timestamp, 
               count(*) reading_count,
               round(sum(temperature)/count(*),2) temperature,
               round(sum(humidity)/count(*),2) humidity,
               round(sum(pressure)/count(*),2) pressure,
               round(sum(altitude)/count(*),2) altitude
            from ha_environment_log
            where sample_timestamp > date_trunc('day', (current_timestamp - INTERVAL '1 day'))
            group by sensor_location, date_trunc('hour', sample_timestamp) order by sample_timestamp, sensor_location;
        """)
        logs = cur.fetchall()
        cur.close()
        conn.close()
        return logs

    def getEnvironmentDailyLogSummary(self):
        conn = psycopg2.connect("dbname="+self.database+" user="+self.user+" password="+self.password)
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
            select sensor_location,
               date_trunc('day', sample_timestamp) sample_timestamp, 
               count(*) reading_count,
               round(min(temperature),2) min_temperature,
               round(max(temperature),2) max_temperature,
               round(min(humidity),2) min_humidity,
               round(max(humidity),2) max_humidity,
               round(min(pressure),2) min_pressure,
               round(max(pressure),2) max_pressure
            from ha_environment_log
            where sample_timestamp > date_trunc('day', (current_timestamp - INTERVAL '14 day'))
            group by sensor_location, date_trunc('day', sample_timestamp) order by sample_timestamp , sensor_location;
        """)
        logs = cur.fetchall()
        cur.close()
        conn.close()
        return logs
