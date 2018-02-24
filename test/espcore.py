### low level logging and protocol handling

# normally you shouldnt need to look into this file too much

import logging
import colorlog
import config

colorlog.basicConfig(level=logging.DEBUG)

logging.getLogger("requests.packages.urllib3.connectionpool").setLevel(logging.ERROR)
