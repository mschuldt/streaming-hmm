files = ["square2", "up_down", "z", "roll_flip"]

out = open("gestures.c", "w");

out.write('#include "hmm.h"\n');
l = len(files)
out.write("int n_gestures = {};\n".format(l))
out.write("gesture** gestures = (gesture**)malloc(sizeof(gesture*)*{});\n".format(l))

out.write("gesture* g;\n")
out.write("void init_gestures(){\n");
out.write("double *tmp;\n")
fmt = "../we-g/gesture_recordings/{}/14.csv"
for i,name in enumerate(map(lambda x : fmt.format(x), files)):
    en = open(name)
    out.write("\n// {}\n".format(name))    
    out.write("g = gestures[{}] = (gesture*)malloc(sizeof(gesture));\n".format(i))
    lines = en.readlines()[1:]
    l = len(lines)
    out.write("g->len = {};\n".format(l));
    out.write("g->data = (double**)malloc(sizeof(double**)*{});\n".format(l));
    for n, line in enumerate(lines):
        out.write("tmp = g->data[{}] = (double*)malloc(sizeof(double)*3);\n".format(n))
        for nn, val in enumerate(line.strip().split(",")[1:]):
            out.write("tmp[{}] = {};\n".format(nn, val))
out.write("}\n");
out.close()
