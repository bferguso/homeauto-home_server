import sys
import logging
import os
from dotenv import load_dotenv
logging.basicConfig(stream=sys.stderr)
sys.path.insert(0,"/var/www/python")
project_folder = os.path.expanduser('~/env')
load_dotenv(os.path.join(project_folder, 'homeServer.env'))
from homeServer.app import app as application