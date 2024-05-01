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

# f = open(assetsJson, "wb+");
# fStruct = open(structPath, "wb+");

def getTexture(obj):
    if len(obj.material_slots) > 0:
        material = obj.material_slots[0].material
        if material and material.use_nodes:
            tree = material.node_tree;
            nodes = tree.nodes;
            
            # Iterate through material nodes to find the image texture node
            for node in nodes:
                if node.type == 'TEX_IMAGE':
                    return node.image;

class Vertex:
    def __init__(self, co, uv):
        self.x = co.x;
        self.y = co.y;
        self.z = co.z;

        self.uvX = uv.x;
        self.uvY = uv.y;

        print(f"{self.x} {self.y} {self.z} => {self.uvX} {self.uvY}");

    def data(self):
        return [self.x, self.y, self.z, self.uvX, self.uvY];
        # vertexIndex = v.vertex_index;
        
        # print("Vertex Index:", vertexIndex);
        # print("UV Coordinates:", uv);

class Item:
    def __init__(self, modelName, index, showDebug=False):
        self.name = modelName;
        self.index = index;
        self.obj = bpy.context.scene.objects.get(self.name);
        self.name = self.obj.name;
        self.showDebug = showDebug;
        
        self.getData();

    def debug(self, *args):
        if(self.showDebug):
            print(*args);

    def __str__(self):
        return f"{self.name} {len(self.data)} {self.texturePath}";

    def export(self):
        file = os.path.join(blocksDir, self.name);
        with open(file, "wb") as f:
            data = [];
            # print(self.data);
            for face in self.data:
                for v in face:
                    for d in v.data():
                        data.append(d);
            # data = np.array(self.data).flatten();
            # print(data);
            # print("{} => {} {:.4f} {:.4f} {:.4f}".format(self.mesh.name, len(data), x, y, z));
            # print(f"{self.name}[{len(data)}]");
            # print(data);
            print(self.index);
            f.write(struct.pack("i", self.index));
            f.write(struct.pack("i", len(data)));
            f.write(struct.pack(f"{len(data)}f", *data));
            self.debug(f"exported {self.name}");

    def getData(self):
        # self.print(model);
        # col = bpy.data.collection.get();
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
        faces = [];
        for i, face in enumerate(bm.faces):
            # print(f"trig {i}");
            vertices = [];
            for loop in face.loops:
            # for v in face.verts:
                uv = loop[uv_lay].uv;

                # print("Loop UV: %f, %f" % uv[:]);
                vert = loop.vert;
                # print("Loop Vert: (%f,%f,%f)" % vert.co[:]);

                # vertex = Vertex(v.co, uv_layer[v.index].uv);
                # print(loop);
                # print(uv);
                vertex = Vertex(vert.co, uv);
                vertices.append(vertex);
            faces.append(vertices);
            
        del(evalObj);
        bm.free();
        
        self.data = faces;
        self.texture = getTexture(self.obj);
        self.texturePath = self.texture.filepath;
        self.debug(f"texture from {self.obj.name} {self.texturePath}");

# a wrapper around division so i can "devide" by zero
def devide(n, d):
    return n / d if d else 0;

def translate(v, fromMin, fromMax, toMin, toMax):
    fromSize = fromMax - fromMin;
    toSize = toMax - toMin;

    scaled = float(v - fromMin) / float(fromSize);
    return toMin + (scaled * toSize);

def joinTextures(imagesPaths, items):
    print("===================== TEXTURES =====================");
    images = [];
    count = 0;
    _imagesPaths = [];
    for imgPath in imagesPaths:
        # print(folder);
        fullPath = folder + imgPath
        print(fullPath);
        img = PIL.Image.open(fullPath);
        images.append(img);
        _imagesPaths.append(imgPath);
        w, h = img.size;
        x = w / 16;
        y = h / 16;
        count += x * y;
        print(w, h);

    # make from set an array
    imagesPaths = _imagesPaths;
    print(imagesPaths);

    print(count);
    minSize = math.ceil(math.sqrt(count+1));
    # opengl has min of 64 size for texture (64 / 16 = 4);
    minSize = max(4, minSize);

    imgSize = minSize * 16;
    defVal = np.array([191.25, 0, 204.0, 255], dtype=np.uint8);
    image = np.full((imgSize, imgSize, 4), defVal, dtype=np.uint8);

    for y in range(imgSize):
        for x in range(imgSize):
            image[y, x] = [int(255 * x / imgSize), int(255 * y / imgSize), 0, 255];

    texturesPath = [];
    textures = [];

    # create a map where eatch image will point to the map of textures
    imageToTextreMap = dict({});
    
    dir = os.path.join(folder, "Output");
    for imgIndex, img in enumerate(images):
        w, h = img.size;
        imgData = np.array(img);
        sw = int(w / 16);
        sh = int(h / 16);
        # print(f"sizes {sw} {sh}");
        # loop throw all the textures on the curr image
        # 1 | 2 | 3 | ...
        # ---------------
        # 4 | 5 | 6 | 
        
        # create texure map, eatch texture will have mapped the start and the end
        # eg 5 text in image above would be [[wholeHeight / y * height, whileWidth / x * widht], ]
        map = [];
        textMap = [];
        for y in range(sh):
            for x in range(sw):
                texturesPath.append(imagesPaths[imgIndex]);
                data = np.zeros((16, 16, 4), dtype=np.uint8);
                # print(y, x);
                # copy the 1 texture (16, 16)
                startY = devide((y * 16), h);
                startX = devide((x * 16), w);

                endY = ((y + 1) * 16) / h;
                endX = ((x + 1) * 16) / w;
                map.append([[startX, startY], [endX, endY]]);

                fullTextureIndex = len(textures);
                fTX = fullTextureIndex % minSize;
                fTY = int(fullTextureIndex / minSize);
                # print("pos", fTX, fTY);
                
                tStartY = devide((fTY * 16), imgSize);
                tStartX = devide((fTX * 16), imgSize);

                tEndX = ((fTX + 1) * 16) / imgSize;
                tEndY = ((fTY + 1) * 16) / imgSize;

                textMap.append([[tStartX, tStartY], [tEndX, tEndY]]);

                for i in range(16):
                    for j in range(16):
                        data[i,j] = imgData[y * 16 + i, x * 16 + j];
                
                png = Image.fromarray(data, "RGBA");
                output = os.path.join(dir, f"{len(textures)}.png");
                # print(output);
                png.save(output);
                
                # data = np.fliplr(data);
                data = np.flipud(data);
                textures.append(data);

        # print(f"texture map: ", map);
        textureMap = {};
        textureMap["length"] = [sw, sh];
        textureMap["size"] = sw * sh;
        textureMap["maps"] = map;
        textureMap["textureMap"] = textMap;
        for mapI in range(len(map)):
            print(f"{map[mapI]} => {textMap[mapI]}");
        # print(textMap);
        imageToTextreMap[imagesPaths[imgIndex]] = textureMap;

    textureIndex = 0;
    prev = None;
    for y in range(minSize):
        for x in range(minSize):
            if(count <= textureIndex):
                break;
            # copy the texture
            for i in range(16):
                for j in range(16):
                    image[y * 16 + i, x * 16 + j] = textures[textureIndex][i,j];
            # print("pixels from {}{} to {}{}");
            print(f"new texture on {x}:{y}");

            # move all uv cords so they would match
            for item in items:
                if(item.texturePath == texturesPath[textureIndex] and item.texturePath != prev):
                    prev = item.texturePath
                    tMap = imageToTextreMap[item.texturePath];
                    maps = tMap["maps"];
                    textMap = tMap["textureMap"];
                    # print(maps);
                    [w, h] = tMap["length"];
                    # move all
                    # print(item.texturePath);
                    for j, face in enumerate(item.data):
                        # print(f);
                        for i, d in enumerate(face):
                            # no images with widht of y != 1
                            index = min(int(d.uvX * w), len(maps) - 1);
                            # print(f"{d.uvX} {d.uvY} {w} {h} index => {index}");
                            map = maps[index];
                            toMap = textMap[index];
                            item.debug(toMap);
                            # for mi, m in enumerate(maps):
                            #     print(f"{m[0][0]} <= {xUv} and {xUv} <= {m[1][0]}");
                            #     if(m[0][0] <= xUv and xUv <= m[1][0]):
                            #         print("match", mi, index);
                            # mapMinX = devide(1, x * 16); mapMaxX = devide((x + 1) * 1);
                            
                            newXVal = translate(d.uvX, map[0][0], map[1][0], toMap[0][0], toMap[1][0]);
                            newYVal = translate(d.uvY, map[0][1], map[1][1], toMap[0][1], toMap[1][1]);
                            
                            # newXVal = devide(translate(d.uvX, map[0][0], map[1][0], x * 16, (x + 1) * 16), imgSize);
                            # newYVal = devide(translate(d.uvY, map[0][1], map[1][1], y * 16, (y + 1) * 16), imgSize);
                            item.debug(f"uv {d.uvX} {d.uvY} => {newXVal} {newYVal}");
                            item.data[j][i].uvX = newXVal;
                            item.data[j][i].uvY = newYVal;

            textureIndex += 1;

    with open(os.path.join(folder, "texture.bin"), "wb") as f:
        metadata = np.array([imgSize, imgSize], dtype=np.uint8);
        metadata.tofile(f);
        image.tofile(f);
        # print("texture was saved");

    # image = np.flipud(image);
    png = Image.fromarray(image, "RGBA");
    output = os.path.join(dir, f"res.png");
    png.save(output);


items = [];
# set to store all the textures without the duplicates
textures = set({});
# every texture will have and array in which there will be every model that is using it
textToModel = dict({});

# bpy.ops.object.select_all(action='DESELECT');

items = [];
def getObjects():
    debugs = [item.name for item in bpy.context.selected_objects];
    print(debugs);
    for itemName in models:
        model = models[itemName];
        name = model["modelName"];
        item = Item(model["modelName"], model["index"], name in debugs);
        items.append(item);

        print(item);
        if(item.texturePath not in textures):
            textures.add(item.texturePath);
            # textures[item.texturePath] = len(textures);

getObjects();
joinTextures(textures, items);

for item in items:
    item.export();
print(f"number of textures {len(textures)}");
