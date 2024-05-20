import bpy, bmesh
import struct
import string
import os

folder = bpy.path.abspath("//");
textFile = os.path.join(folder, "text.bin");

class Letter:
    def __init__(self, letter):
        self.letter = letter;
        bpy.ops.object.select_all(action='DESELECT');

        bpy.ops.object.text_add(location=(0,0,0));
        self.textObj = bpy.context.object;
        self.textObj.data.body = letter;

        self.textObj.data.size = 1.0;
        self.textObj.data.font = bpy.data.fonts['RobotoMono Nerd Font Mono Regular'];

        bpy.ops.object.convert(target='MESH');

        self.obj = bpy.context.object;
        self.obj.name = letter;

        self.getData();

    def export(self, f):
        # https://docs.python.org/3/library/struct.html#struct-format-strings
        f.write(struct.pack("B", ord(self.letter)));
        f.write(struct.pack("I", len(self.data)));
        f.write(struct.pack(f"{len(self.data)}f", *self.data));
        print(f"exported {self.letter} with size {len(self.data)}");

        # self.obj.select_set(True);
        # bpy.ops.object.delete();

    def getData(self):
        # mesh = self.obj.data;
        # print(mesh);

        mesh = self.obj.data;
        graph = bpy.context.evaluated_depsgraph_get();
        evalObj = mesh.evaluated_get(graph);
        meshData = self.obj.data;
        
        bm = bmesh.new();
        bm.from_mesh(meshData);
        
        bmesh.ops.triangulate(bm, faces=bm.faces[:]);

        data = [];
        for face in bm.faces:
            for loop in face.loops:
                vert = loop.vert;
                for v in vert.co:
                    data.append(v);

                data.append(0);
                data.append(0);

        del(evalObj);
        bm.free();

        self.data = data;

        bpy.ops.object.delete(use_global=False, confirm=False);

with open(textFile, "wb") as f:
    letters = [];
    size = 0;
    for i in range(33, 127):
        c = chr(i)
        if(c in string.printable):
            print(i);
            l = Letter(c);
            letters.append(l);
            size += len(l.data);

    print(size);
    f.write(struct.pack("B", len(letters)));
    f.write(struct.pack("I", size));
    for l in letters:
        l.export(f);
