#basic stuff needed in each test

#in each test just do: from esptest import *

#look in this file for basic functions to use in your tests

# from espeasy import *
from node import *
from espeasy import *

import config
import time
import shelve
import os
from espcore import *


### create node objects and espeasy objects
node=[]
espeasy=[]

for n in config.nodes:
    node.append(Node(n, "node"+str(len(node))))
    espeasy.append(EspEasy(node[-1]))


steps=[]

### keep test state, so we can skip tests.
state={
    'module': None,
    'name': None
}
with shelve.open("test.state") as shelve_db:
    if 'state' in shelve_db:
        global state
        state=shelve_db['state']


def step(step):
    """add test step. test can resume from every test-step"""
    if state['module'] and ( state['module'] != step.__module__ or state['name'] != step.__name__):
        log.debug("Skipping step "+step.__module__ + "." + step.__name__)
    else:
        steps.append(step)
        # add the rest of the steps as well
        state['module']=None


### run all the tests
def run():
    """run all tests"""

    for step in steps:
        print()
        log.info("*** Starting "+step.__module__ + "." + step.__name__ +": "+str(step.__doc__))

        # store this step so we may resume later
        state['module']=step.__module__
        state['name']=step.__name__
        with shelve.open("test.state") as shelve_db:
            shelve_db['state']=state

        #run the step. if there is an exception we resume this step the next time
        step()


        # log.info("Completed step")

    # all Completed
    os.unlink("test.state")
    log.info("*** All tests complete ***")


### auxillary test functions

def test_in_range(value, min, max):

    if value < min or value > max:
        raise("Value {value} should be between {min} and {max}".format(value=value, min=min, max=max))
