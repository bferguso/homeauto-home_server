import urllib.request
import urllib.parse
import json


class TempestDao:
    def __init__(self):
        pass

    # Forecast Locations
    STATION_ID = "41942"
    STATION_TOKEN = "7f162ecd-bb34-49a0-9a3d-985e348d861a"
    GET_STATION_INFO_URL = "https://swd.weatherflow.com/swd/rest/stations?token={token}"
    GET_OBSERVATIONS_URL = "https://swd.weatherflow.com/swd/rest/observations/station/{station_id}?token={token}"

    def get_station_info(self):
        url = self.GET_STATION_INFO_URL.replace("{token}",self.STATION_TOKEN)
        return TempestDao.__get_json_data__(url)

    def get_observations(self):
        url = self.GET_OBSERVATIONS_URL.replace("{station_id}", self.STATION_ID).replace("{token}",self.STATION_TOKEN)
        return TempestDao.__get_json_data__(url)

    @staticmethod
    def __get_json_data__(url):
        response = urllib.request.urlopen(url)
        raw_data = response.read()
        encoding = response.info().get_content_charset('utf8')
        return json.loads(raw_data.decode(encoding))

    def main(self):
        self.get_observations()
        self.get_station_info()

if __name__ == "__main__":
    TempestDao().main()