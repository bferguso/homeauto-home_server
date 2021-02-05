import psycopg2
import psycopg2.extras
import simplejson as json
import os


class HomeServerDao:
    def __init__(self):
        pass

    user = os.getenv("DB_SCHEMA")
    password = os.getenv("DB_PASSWORD")
    database = os.getenv("DB_DATABASE")

    def get_connection(self):
        return psycopg2.connect("dbname="+self.database+" user="+self.user+" password="+self.password)

    def get_cursor(self, connection):
        cur = connection.cursor()
        cur.execute("SET search_path TO " + self.user)
        return cur

    def set_node_active(self, location_name, ip_address, active):
        conn = self.get_connection()
        cur = self.get_cursor(conn)
        cur.execute("update ha_remote_devices set node_active = %s where node_location = %s and remote_address =  %s;"
                    , (active, location_name, ip_address))
        conn.commit()
        cur.close()
        conn.close()

    def __log_device_seen(self, cur, env_data):
        cur.execute("select * from ha_remote_devices where node_location = %s and remote_address =  %s;"
                    , (env_data['location'], env_data['remote_address']))
        if cur.rowcount == 0:
            cur.execute("insert into ha_remote_devices (node_location, remote_address, last_seen_timestamp ) values( %s, %s, current_timestamp );"
                        , (env_data['location'], env_data['remote_address']))
        else:
            cur.execute("update ha_remote_devices set last_seen_timestamp = current_timestamp, node_active=true where node_location = %s and remote_address =  %s;"
                        , (env_data['location'], env_data['remote_address']))

    def register_node(self, node_data):
        with self.get_connection() as conn:
            with self.get_cursor(conn) as cur:
                print("Trying to save "+str(node_data))
                cur.execute("select * from ha_remote_devices where node_location = %s and remote_address =  %s;"
                            , (node_data['location'], node_data['remote_address']))
                if cur.rowcount == 0:
                    cur.execute("insert into ha_remote_devices (node_location, remote_address, node_active, last_seen_timestamp, node_capabilities_json) values( %s, %s, %s, current_timestamp, %s);"
                                , (node_data['location'], node_data['remote_address'], True, json.dumps(node_data['capabilities'])))
                else:
                    cur.execute("update ha_remote_devices set last_seen_timestamp = current_timestamp, node_capabilities_json = %s, node_active=true where node_location = %s and remote_address =  %s;"
                                , (json.dumps(node_data['capabilities']), node_data['location'], node_data['remote_address']))

    def save_environment_log(self, env_data):
        with self.get_connection() as conn:
            with self.get_cursor(conn) as cur:
                pressure = env_data['pressure'] if 'pressure' in env_data else 0.00
                alt = env_data['alt'] if 'alt' in env_data else 0.00
                heatIndex = env_data['heatIndex'] if 'heatIndex' in env_data else 0.00
                if 'sample_timestamp' not in env_data or not env_data['sample_timestamp']:
                    cur.execute("insert into ha_environment_log(node_location,temperature,humidity, pressure, altitude, heat_index) values (%s, %s, %s, %s, %s, %s);"
                            , (env_data['location'], env_data['temp'], env_data['humidity'], pressure, alt, heatIndex))
                else:
                    cur.execute("insert into ha_environment_log(node_location,temperature,humidity, pressure, altitude, heat_index, sample_timestamp) values (%s, %s, %s, %s, %s, %s, %s);"
                            , (env_data['location'], env_data['temp'], env_data['humidity'], pressure, alt, heatIndex, env_data['sample_timestamp']))
                self.__log_device_seen(cur, env_data)

    def get_last_seen_devices(self):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("select node_location, remote_address, last_seen_timestamp, node_active, node_capabilities_json from ha_remote_devices order by last_seen_timestamp desc;");
        last_log = cur.fetchall()
        cur.close()
        conn.close()
        return last_log

    def get_last_environment_log(self, location):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
            select node_location,
                   temperature,
                   humidity, 
                   pressure,
                   sample_timestamp 
            from ha_environment_log
            where node_location = %(location)s 
                and sample_timestamp =
                    (select max(sample_timestamp) from ha_environment_log
                        where node_location = %(location)s)
            order by sample_timestamp desc;
        """,
                    {'location': location})
        last_log = cur.fetchone()
        cur.close()
        conn.close()
        return last_log

    def get_hourly_times(self):
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

    def get_daily_times(self):
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

    def get_locations_in_period(self):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
            select distinct node_location
                from ha_environment_log
                where sample_timestamp > date_trunc('day', (current_timestamp - INTERVAL '1 day'))
                order by node_location;
                """)
        locations = cur.fetchall()
        cur.close()
        conn.close()
        return locations

    def get_environment_log_summary(self, location):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
        with all_hours as ( select distinct date_trunc('hour', sample_timestamp) sample_timestamp
                    from ha_environment_log
                    where sample_timestamp > date_trunc('day', (current_timestamp - INTERVAL '1 day'))
                    order by sample_timestamp),
         env_data as (
             select node_location,
               date_trunc('hour', sample_timestamp) sample_timestamp, 
               count(*) reading_count,
               round(sum(temperature)/count(*),2) temperature,
               round(sum(humidity)/count(*),2) humidity,
               round(sum(pressure)/count(*),2) pressure,
               round(sum(altitude)/count(*),2) altitude 
         from ha_environment_log
         where  node_location = %(location)s 
         and ha_environment_log.sample_timestamp  > date_trunc('day', (current_timestamp - INTERVAL '1 day'))
         group by node_location, date_trunc('hour', sample_timestamp)
         order by node_location, date_trunc('hour', sample_timestamp)) 
            select  all_hours.sample_timestamp,
                        %(location)s node_location,
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

    def get_environment_daily_log_summary(self, location):
        conn = self.get_connection()
        cur = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
        cur.execute("""
            with all_days as (
            select distinct date_trunc('day', sample_timestamp) sample_timestamp
                from ha_environment_log
                where sample_timestamp > date_trunc('day', (current_timestamp - INTERVAL '14 day'))
                order by sample_timestamp),
     env_data as (                
            select node_location,
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
            and node_location = %(location)s
            group by node_location, date_trunc('day', sample_timestamp) order by sample_timestamp , node_location)
    select all_days.sample_timestamp,
           %(location)s node_location,
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
