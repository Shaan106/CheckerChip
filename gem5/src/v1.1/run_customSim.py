import m5
from m5.objects import *

root = Root(full_system = False)

root.customSimObj = CustomSimObject(time_to_wait = '2us',
                                    number_of_fires = 9)

# can also do this:
# root.customSimObj = CustomSimObject()
# root.customSimObj.time_to_wait = '2us'
# root.customSimObj.number_of_fires = 9

m5.instantiate()

print("Beginning simulation!")
exit_event = m5.simulate()
print('Exiting @ tick {} because {}'
      .format(m5.curTick(), exit_event.getCause()))