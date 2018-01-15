#basic stuff needed in each test

#in each test just do: from esptest import *


# from espeasy import *
from node import *
from espeasy import *

import config
import time
from espcore import *


### create node objects and espeasy objects
node=[]
espeasy=[]

for n in config.nodes:
    node.append(Node(n, "node"+str(len(node))))
    espeasy.append(EspEasy(node[-1]))
