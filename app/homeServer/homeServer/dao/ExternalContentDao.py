import feedparser
from bs4 import BeautifulSoup
import urllib.request
import urllib.parse
import re

class ExternalContentDao:
    def __init__(self):
        pass

    # Forecast Locations
    HOWE_SOUND = "HOWE_SOUND"
    GEORGIA_NORTH = "GEORGIA_NORTH"
    GEORGIA_SOUTH = "GEORGIA_SOUTH"

    # Marine Condition Stations
    PAM_ROCKS = "PAM_ROCKS"
    POINT_ATKINSON = "POINT_ATKINSON"
    ENGLISH_BAY = "ENGLISH_BAY"
    HALIBUT_BANK = "HALIBUT_BANK"
    GIBSONS = "GIBSONS"

    URLS = {
        "FORECAST": "https://weather.gc.ca/rss/marine/{area}_e.xml",
        "CONDITIONS": "https://weather.gc.ca/marine/incs/weather_conditions_ajax.php?mapID=02&siteID={area}&stationID={station}&lang=e&page=cc",
        # Date format for tides is "YYYY/MM/DD"
        # pres=? 0=>graph, 1=>HTML table, 2=> text
        "TIDES": "https://tides.gc.ca/eng/station?type=0&date={date}&sid={station_id}&tz=PST&pres=2"
    }
    CONDITION_PAGES = {
        "CURRENT_CONDITIONS": "cc",
        "PAST_24H_CONDITIONS": "p24hc",
    }

    AREA_IDS = {
        "HOWE_SOUND": {"id": "06400", "rss_id": "06400", "display_name": "Howe Sound"},
        "GEORGIA_NORTH": {"id": "14301", "rss_id": "14300", "display_name": "Straight of Georgia N."},
        "GEORGIA_SOUTH": {"id": "14305", "rss_id": "14300", "display_name": "Straight of Georgia S."}
    }

    STATION_IDS = {
        "PAM_ROCKS": {"id": "WAS", "display_name": "Pam Rocks", "area_name": "HOWE_SOUND"},
        "POINT_ATKINSON": {"id": "WSB", "display_name": "Point Atkinson", "area_name": "GEORGIA_SOUTH"},
        "ENGLISH_BAY": {"id": "46304",  "display_name": "English Bay", "area_name": "GEORGIA_SOUTH"},
        "HALIBUT_BANK": {"id": "46146", "display_name": "Halibut Bank", "area_name": "GEORGIA_SOUTH"}
    }

    TIDE_STATION_IDS = {
        "POINT_ATKINSON": {"id": "7795", "display_name": "Point Atkinson"},
        "GIBSONS": {"id": "7820", "display_name": "Gibsons"}
    }
    TIDE_DATE_FORMAT = "%Y/%m/%d"


    def getMarineForecast(self, location):
        site = self.__getInfoForLocation(location, self.AREA_IDS)
        forecast = {"location": site["display_name"], "forecastEntries": []}
        NewsFeed = feedparser.parse(self.URLS["FORECAST"].replace("{area}", site["rss_id"]))
        # print ('Number of entries'+str(len(NewsFeed.entries)))
        # print (NewsFeed)
        # print (entry.keys())
        forecast["forecastEntries"].extend(NewsFeed.entries)
        return forecast

    def getMarineConditions(self, station_name):
        station = self.__getInfoForLocation(station_name, self.STATION_IDS)
        site = self.__getInfoForLocation(station["area_name"], self.AREA_IDS)
        conditions = {"location": site["display_name"], "station": station["display_name"], "conditionEntries": []}
        response = urllib.request.urlopen(self.URLS["CONDITIONS"].replace("{area}", site["id"]).replace("{station}", station["id"]))
        raw_data = response.read()
        encoding = response.info().get_content_charset('utf8')
        # print(encoding)
        # print(type(response))
        conditions["conditionEntries"].extend(self.__parseConditions(raw_data.decode(encoding)))
        return conditions

    def get_tides(self, station_name, start_date):
        station = self.__getInfoForLocation(station_name, self.TIDE_STATION_IDS)
        tide_data = {"location": station["display_name"], "station_id": station["id"], "timezone": "PST", "tide_entries": []}
        url = self.URLS["TIDES"].replace("{station_id}", station["id"]).replace("{date}", urllib.parse.quote(start_date.strftime(self.TIDE_DATE_FORMAT)))
        print("Getting url: "+url)
        response = urllib.request.urlopen(url)
        raw_data = response.read()
        encoding = response.info().get_content_charset('utf8')
        tide_data["tide_entries"] = ExternalContentDao.__parse_tides(raw_data.decode(encoding))
        return tide_data

    def __getInfoForLocation(self, location, location_list):
        site = location_list[location]
        if not site:
            raise Exception("Invalid location "+location)
        return site

    def __parseConditions(self, html_conditions):
        data = []
        soup = BeautifulSoup(html_conditions, "html.parser")
        for row in soup.find("table").find("tbody").find_all("tr"):
            key = row.find("th");
            while key:
                value = key.find_next_sibling("td")
                data.append({"title": key.renderContents().decode().replace("\xa0"," "), "value": value.text})
                key = value.find_next_sibling("th")
            #data.append(row)
        return data

    @staticmethod
    def __parse_tides(html_tides):
        data = []
#        print("Trying to parse "+html_tides)
        soup = BeautifulSoup(html_tides, "html.parser")

        tide_lines = soup.find("div", "stationTextData").find_all("div")

        print(tide_lines)
        for tide_line in tide_lines:
            text = tide_line.renderContents().decode()
            parts = text.strip().split(";")
            if len(parts) == 4:
                tide_entry = {"time": parts[0].strip()+"T"+parts[1], "height_m": re.sub(r"\(.*$", "", parts[2]), "height_ft": re.sub(r"\(.*$", "", parts[3].strip())}
                data.append(tide_entry)
            else:
                print("Unknown tide data format "+tide_line)
        print(data)
        return data

#Conditions: https://weather.gc.ca/marine/incs/weather_conditions_ajax.php?mapID=02&siteID=06400&stationID=WAS&lang=e&page=cc
# Pages: cc = current conditions
#        p24hc = previous 24h conditions
#    var page = function(){
#    if(href()[0].indexOf('currentConditions') !== -1){
#    return 'cc';
#    }else if(href()[0].indexOf('24hrObsHistory') !== -1){
#    return 'p24hc';
#}else if(href()[0].indexOf('regionalSummary') !== -1){
#return 'rs';
#}else if(href()[0].indexOf('lightstation') !== -1){
#return 'ls';
#}
#
#return false;
#};
#
#// sends an ajax request to weather_conditions_ajax.php
#                            // on success, clear the weather conditions tab and load the returned html
#$.ajax({
#    type: 'GET',
#    cache: false,
#    url: '/marine/incs/weather_conditions_ajax.php?' + href()[1] + '&lang=' + lang + '&page=' + page(),
#}).done(function(html){
#    contentContainer.empty();
#contentContainer.append(html);
#
#$('html, body').animate({ scrollTop : $('#weather-conditions-lnk').offset().top }, 'slow');
#
#var baseWidth = 590, \
#                baseCoords = [];
#
#$('img[usemap]').each(function(){
#    var thisImg = $(this), \
#                   currentWidth = thisImg.width(), \
#                                  offset = parseFloat((currentWidth / baseWidth).toFixed(4));
#
#$(thisImg.attr('usemap')).children().each(function(i){
#    var thisArea = $(this);
#
#var coords = thisArea.attr('coords').split(',');
#
#baseCoords[i] = [
#    coords[0],
#    coords[1],
#    coords[2],
#];
#
#var x = Math.round(parseInt(baseCoords[i][0]) * offset), \
#        y = Math.round(parseInt(baseCoords[i][1]) * offset);
#
#thisArea.attr('coords', x + ',' + y + ',' + baseCoords[i][2]);
#});
#});
#});