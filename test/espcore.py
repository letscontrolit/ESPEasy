
import logging
import colorlog
colorlog.basicConfig(level=logging.DEBUG)
log=logging.getLogger("TEST")


logging.getLogger("requests.packages.urllib3.connectionpool").setLevel(logging.ERROR)
