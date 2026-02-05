import json

with open('Opcodes.json', 'r') as file:
    data = json.load(file)

unprefixed = data["unprefixed"]

prefixed = data["cbprefixed"]

for i in unprefixed:
    if len(unprefixed[i]["cycles"])>1:
        base = unprefixed[i]["cycles"][1]
        taken = unprefixed[i]["cycles"][0]
    else:
        base = unprefixed[i]["cycles"][0]
        taken = 0
    #print("{" + str(i) + ", " + str(unprefixed[i]["bytes"]) + ", " + str(base) + ", " + str(taken) + "},")

for i in prefixed:
    base = prefixed[i]["cycles"][0]
    taken = 0
    print("{" + str(i) + ", " + str(prefixed[i]["bytes"]) + ", " + str(base) + ", " + str(taken) + "},")

