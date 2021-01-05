import psycopg2
import psycopg2.extras


class HomeServerDao:
    def __init__(self):
        pass

    user = "homeauto"
    password = "homeauto"
    database = "homeauto"

    def get_connection(self):
        return psycopg2.connect("dbname="+self.database+" user="+self.user+" password="+self.password)

    def logDeviceSeen(self, cur, envData):
        cur.execute("select * from public.ha_remote_devices where sensor_location = %s and remote_address =  %s;"
                    , (envData['location'], envData['remote_address']))
        if cur.rowcount == 0:
            cur.execute("insert into public.ha_remote_devices (sensor_location, remote_address, last_seen_timestamp ) values( %s, %s, current_timestamp );"
                        , (envData['location'], envData['remote_address']))
        else:
            cur.execute("update public.ha_remote_devices set last_seen_timestamp = current_timestamp where sensor_location = %s and remote_address =  %s;"
                        , (envData['location'], envData['remote_address']))

    def saveEnvironmentLog(self, envData):
        conn = self.get_connection()
        cur = conn.cursor()
        pressure = envData['pressure'] if 'pressure' in envData else 0.00
        alt = envData['alt'] if 'alt' in envData else 0.00
        heatIndex = envData['heatIndex'] if 'heatIndex' in envData else 0.00
        cur.execute("insert into public.ha_environment_log(sensor_location,temperature,humidity, pressure, altitude, heat_index) values (%s, %s, %s, %s, %s, %s);"
                    , (envData['location'], envData['temp'], envData['humidity'], pressure, alt, heatIndex))
        self.logDeviceSeen(cur, envData)
        conn.commit()
        cur.close()
        conn.close()

    def getLastSeenDevices(self):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("select sensor_location, remote_address, last_seen_timestamp, device_active from public.ha_remote_devices order by last_seen_timestamp desc;");
        lastLog = cur.fetchall()
        cur.close()
        conn.close()
        return lastLog

    def getLastEnvironmentLog(self, location):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
            select sensor_location,
                   temperature,
                   humidity,
                   pressure,
                   sample_timestamp
             from public.ha_environment_log where sensor_location = %(location)s order by sample_timestamp desc;
        """,
                    {'location': location})
        lastLog = cur.fetchone()
        cur.close()
        conn.close()
        return lastLog

    def getHourlyTimes(self):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
            select distinct date_trunc('hour', sample_timestamp) sample_timestamp
                from ha_environment_log
                where sample_timestamp > date_trunc('day', (current_timestamp - INTERVAL '1 day'))
                order by sample_timestamp;
                """)
        times = cur.fetchall()
        cur.close()
        conn.close()
        return times

    def getDailyTimes(self):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
            select distinct date_trunc('day', sample_timestamp) sample_timestamp
                from ha_environment_log
                where sample_timestamp > date_trunc('day', (current_timestamp - INTERVAL '14 day'))
                order by sample_timestamp;
                """)
        times = cur.fetchall()
        cur.close()
        conn.close()
        return times

    def getLocationsInPeriod(self):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
            select distinct sensor_location
                from ha_environment_log
                where sample_timestamp > date_trunc('day', (current_timestamp - INTERVAL '1 day'))
                order by sensor_location;
                """)
        locations = cur.fetchall()
        cur.close()
        conn.close()
        return locations

    def getEnvironmentLogSummary(self, location):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
        with all_hours as ( select distinct date_trunc('hour', sample_timestamp) sample_timestamp
                    from ha_environment_log
                    where sample_timestamp > date_trunc('day', (current_timestamp - INTERVAL '1 day'))
                    order by sample_timestamp),
         env_data as (
             select sensor_location,
               date_trunc('hour', sample_timestamp) sample_timestamp, 
               count(*) reading_count,
               round(sum(temperature)/count(*),2) temperature,
               round(sum(humidity)/count(*),2) humidity,
               round(sum(pressure)/count(*),2) pressure,
               round(sum(altitude)/count(*),2) altitude 
         from ha_environment_log
         where  sensor_location = %(location)s 
         and ha_environment_log.sample_timestamp  > date_trunc('day', (current_timestamp - INTERVAL '1 day'))
         group by sensor_location, date_trunc('hour', sample_timestamp)
         order by sensor_location, date_trunc('hour', sample_timestamp)) 
            select  all_hours.sample_timestamp,
                        %(location)s sensor_location,
                        env_data.reading_count,
                        env_data.temperature,
                        env_data.humidity,
                        env_data.pressure,
                        env_data.altitude
                        from all_hours
                         FULL OUTER JOIN env_data  ON  env_data.sample_timestamp = all_hours.sample_timestamp;
        """, {'location': location})
        logs = cur.fetchall()
        cur.close()
        conn.close()
        return logs

    def getEnvironmentDailyLogSummary(self, location):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
            with all_days as (
            select distinct date_trunc('day', sample_timestamp) sample_timestamp
                from ha_environment_log
                where sample_timestamp > date_trunc('day', (current_timestamp - INTERVAL '14 day'))
                order by sample_timestamp),
     env_data as (                
            select sensor_location,
               date_trunc('day', sample_timestamp) sample_timestamp, 
               count(*) reading_count,
               round(min(temperature),2) min_temperature,
               round(max(temperature),2) max_temperature,
               round(avg(temperature),2) avg_temperature,
               round(min(humidity),2) min_humidity,
               round(max(humidity),2) max_humidity,
               round(avg(humidity),2) avg_humidity,
               round(min(pressure),2) min_pressure,
               round(max(pressure),2) max_pressure,
               round(avg(pressure),2) avg_pressure
            from ha_environment_log
            where sample_timestamp > date_trunc('day', (current_timestamp - INTERVAL '14 day'))
            and sensor_location = %(location)s
            group by sensor_location, date_trunc('day', sample_timestamp) order by sample_timestamp , sensor_location)
    select all_days.sample_timestamp,
           %(location)s sensor_location,
           reading_count,
           min_temperature, max_temperature, avg_temperature,
           min_humidity, max_humidity, avg_humidity,
           min_pressure, max_pressure, avg_pressure
        from all_days
        FULL OUTER JOIN env_data  ON  env_data.sample_timestamp = all_days.sample_timestamp;
        """, {'location': location})
        logs = cur.fetchall()
        cur.close()
        conn.close()
        return logs