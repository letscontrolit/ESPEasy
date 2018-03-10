#basic stuff needed in each test

#in each test just do: from esptest import *

#look in this file for basic functions to use in your tests

# from espeasy import *
from node import *
from espeasy import *
from controlleremu import *

import config
import shelve
import os
from espcore import *
import argparse
import util

### parse arguments

parser = argparse.ArgumentParser(description='ESPEasy testing framework', formatter_class=argparse.RawDescriptionHelpFormatter)
parser.add_argument('--debug', action='store_true', help='enable http debugging output')
parser.add_argument('--no-resume', action='store_true', help='dont resume testing from where we left off last time')
parser.add_argument('--skip-power', action='store_true', help='skip tests that require powercycling.')
args = parser.parse_args()


if args.debug:
    util.enable_http_debug()


### create node objects and espeasy objects

node=[]
espeasy=[]

for n in config.nodes:
    node.append(Node(n, "node"+str(len(node)), skip_power=args.skip_power))
    espeasy.append(EspEasy(node[-1]))


# steps=[]

log=logging.getLogger("esptest")


## controller emulators
controller=ControllerEmu()

### keep test state, so we can skip tests.
global state
state={
    'module': None,
    'name': None,
    'title':""
}
with shelve.open("test.state") as shelve_db:
    if 'state' in shelve_db:
        state=shelve_db['state']


def step(title=""):
    def step_dec(test):
        """add test step. test can resume from every test-step"""
        if not args.no_resume and state['module'] and ( state['module'] != test.__module__ or state['name'] != test.__name__ or state['title'] != title):
            log.debug("Skipping step "+title+": "+test.__module__ + "." + test.__name__ )
        else:
            state['module']=None
            print()
            log.info("*** Starting step "+title+": "+test.__module__ + "." + test.__name__ )

            # store this test so we may resume later
            with shelve.open("test.state") as shelve_db:
                shelve_db['state']={
                    'module': test.__module__,
                    'name': test.__name__,
                    'title': title
                }


            #run the test. if there is an exception we resume this test the next time
            test()
            log.info("*** Starting step "+title+": "+test.__module__ + "." + test.__name__ )




    return(step_dec)


# ### run all the tests
# def run():
#     """run all tests"""
#
#     for step in steps:
#         print()
#         log.info("*** Starting "+step.title+": "+step.__module__ + "." + step.__name__ )
#
#         # store this step so we may resume later
#         state['module']=step.__module__
#         state['name']=step.__name__
#         state['title']=step.title
#         with shelve.open("test.state") as shelve_db:
#             shelve_db['state']=state
#
#         #run the step. if there is an exception we resume this step the next time
#         step()
#
#
#         # log.info("Completed step")
#
#     # all Completed
#     os.unlink("test.state")
#     log.info("*** All tests complete ***")


### auxillary test functions
def test_in_range(value, min, max):

    if value < min or value > max:
        raise(Exception("Value {value} should be between {min} and {max}".format(value=value, min=min, max=max)))

    log.info("OK: value {value} is between {min} and {max}".format(value=value, min=min, max=max))

def test_is(value, shouldbe):
    if value!=shouldbe:
        raise(Exception("Value {value} should be {shouldbe}".format(value=value, shouldbe=shouldbe)))

    log.info("OK: Value is {value}".format(value=value, shouldbe=shouldbe))


def pause(seconds):
    log.info("Waiting for {seconds} seconds".format(seconds=seconds))
    time.sleep(seconds)



def completed():
    if os.path.exists("test.state"):
        os.unlink("test.state")

    log.info("*** All tests completed ***")
