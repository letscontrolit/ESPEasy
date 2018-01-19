### low level logging and protocol handling

# normally you shouldnt need to look into this file too much

import logging
import colorlog
import config

colorlog.basicConfig(level=logging.DEBUG)


logging.getLogger("requests.packages.urllib3.connectionpool").setLevel(logging.ERROR)




# def mqtt_expect_json(topic, matches, timeout=60):
#     """wait until a specific json message is received, and return it decoded. ignores all other messages"""
#
#     start_time=time.time()
#
#     logging.getLogger("MQTT").info("Waiting for json message on topic {topic}, with values {matches}".format(topic=topic, matches=matches))
#
#     # check mqtt results
#     while time.time()-start_time<timeout:
#         while mqtt_messages:
#             message=mqtt_messages.pop()
#             try:
#                 #ignore decoding exceptions
#                 payload=json.loads(message.payload.decode())
#             except:
#                 continue
#
#             if message.topic == topic:
#                 ok=True
#                 for match in matches.items():
#                     if not match[0] in payload or payload[match[0]]!=match[1]:
#                         ok=False
#                 if ok:
#                     return(payload)
#         time.sleep(1)
#
#     raise(Exception("Timeout while expecting mqtt json message"))




# def http_expect_request(path, matches, timeout=60):
#     """wait until a specific path and paraters are request on the http server. ignores all other requests"""
#
#     start_time=time.time()
#
#     logging.getLogger("HTTP").info("Waiting for http request on path {path}, with values {matches}".format(path=path, matches=matches))
#
#     # check http results
#     while time.time()-start_time<timeout:
#         while True:
#             request=http_requests.get(block=True, timeout=timeout)
#             # logging.getLogger("HTTP").debug("Body: "+str(request.body.str))
#             if request.path == path:
#
#                 ok=True
#                 for match in matches.items():
#                     if not match[0] in request.params or request.params[match[0]]!=match[1]:
#                         ok=False
#                 if ok:
#                     return(request)
#         time.sleep(1)
#
#     raise(Exception("Timeout while expecting http message"))
