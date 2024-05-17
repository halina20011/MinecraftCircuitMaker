import os
import bpy, bmesh
import json
import math
import struct

import sys
import numpy as np
import PIL
from PIL import Image
#SITE_PACKAGES = "/usr/lib/python3.11/site-packages";
#if SITE_PACKAGES not in sys.path:
#    sys.path.insert(0, SITE_PACKAGES);

#from PIL import Image

print("====================================================");
print("===================== STARTING =====================");
print("====================================================");

def readJson(file):
    with open(file, "r") as f:
        d = json.load(f);
        return d;

folder = bpy.path.abspath("//");
assetsJson = os.path.join(folder, "assets.json");
models = readJson(assetsJson);
blocksDir = os.path.join(folder, "Blocks");
if(not os.path.exists(blocksDir)):
    print(f"created {blocksDir}");
    os.makedirs(blocksDir);

def getTexture(obj):
    if len(obj.material_slots) > 0:
        material = obj.material_slots[0].material
        if material and material.use_nodes:
            tree = material.node_tree;
            nodes = tree.nodes;
            
            # Iterate through material nodes to find the image texture node
            for node in nodes:
                if node.type == 'TEX_IMAGE':
                    return node.image.filepath;

    return "";

class Vertex:
    def __init__(self, co, uv):
        self.x = co.x;

        self.uvX = uv.x;
        self.uvY = uv.y;

        # use rotation rule to rotate around x axis -90 deg
        self.y = co.z;
        self.z = -co.y;

    def data(self):
        return [self.x, self.y, self.z, self.uvX, self.uvY]

class Item:
    def __init__(self, modelName, modelId):
        self.name = modelName;
        self.modelId = modelId;
        self.obj = bpy.context.scene.objects.get(self.name);
        self.name = self.obj.name;
        
        self.getData();

    def export(self):
        file = os.path.join(blocksDir, self.name);
        with open(file, "wb") as f:
            # https://docs.python.org/3/library/struct.html#struct-format-strings
            f.write(struct.pack("B", len(self.modelId)));
            f.write(struct.pack(f"{len(self.modelId)}s", self.modelId.encode()));
            f.write(struct.pack("i", len(self.data)));
            f.write(struct.pack(f"{len(self.data)}f", *self.data));
            print(f"exported {self.name}");

    def getData(self):
        # Check if the object is a mesh
        if(self.obj.type != "MESH"):
            return;

        mesh = self.obj.data;

        graph = bpy.context.evaluated_depsgraph_get();
        evalObj = mesh.evaluated_get(graph);
        meshData = self.obj.data;
        
        bm = bmesh.new();
        bm.from_mesh(meshData);
        
        bmesh.ops.triangulate(bm, faces=bm.faces[:]);
        
        if not mesh.uv_layers:
            return
        
        uv_lay = bm.loops.layers.uv.active;

        # uv_layer = mesh.uv_layers.active.data;
        data = [];
        for face in bm.faces:
            for loop in face.loops:
                uv = loop[uv_lay].uv;
                vert = loop.vert;
                for v in Vertex(vert.co, uv).data():
                    data.append(v);

        del(evalObj);
        bm.free();
        
        self.data = data;

TEXTURE_SIZE = 256;

def exportAll(items):
    for item in items:
        item.export()


    fullPath = folder + getTexture(items[0].obj);
    print(fullPath);
    texture = np.array(PIL.Image.open(fullPath));
    rawTexture = os.path.join(folder, "texture.bin");
    print(rawTexture)
    with open(rawTexture, "wb") as f:
        metadata = np.array([TEXTURE_SIZE, TEXTURE_SIZE], dtype=np.uint16);
        metadata.tofile(f);

        texture = np.flipud(texture);
        texture.tofile(f);

items = [];

def printSelected():
    for obj in bpy.context.selected_objects:
        item = Item(obj.name, 0);
        print(item.data);

def getObjects():
    for itemName in models:
        model = models[itemName];
        name = model["modelName"];
        item = Item(name, model["id"]);
        items.append(item);

        print(item.name);

printSelected();
getObjects();
exportAll(items);
