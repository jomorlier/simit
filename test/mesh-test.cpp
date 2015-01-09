#include "gtest/gtest.h"

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <dirent.h>

#include "mesh.h"

using namespace std;
using namespace simit;

TEST(Mesh, MeshlabTest) {
  const string input = R"(####
#
# OBJ File Generated by Meshlab
#
####
# Object cubeb.obj
#
# Vertices: 8
# Faces: 12
#
####
v 0.000000 0.000000 0.000000
v 0.010000 0.000000 0.000000
v 0.010000 0.000000 0.010000
v 0.000000 0.000000 0.010000
v 0.000000 0.100000 0.000000
v 0.010000 0.100000 0.000000
v 0.010000 0.100000 0.010000
v 0.000000 0.100000 0.010000
# 8 vertices, 0 vertices normals

f 1 2 4
f 2 3 4
f 6 1 5
f 6 2 1
f 7 2 6
f 7 3 2
f 4 3 7
f 4 7 8
f 5 1 4
f 4 8 5
f 8 6 5
f 8 7 6
# 12 faces, 0 coords texture

# End of File)";

  const string output = R"(v 0 0 0
v 0.01 0 0
v 0.01 0 0.01
v 0 0 0.01
v 0 0.1 0
v 0.01 0.1 0
v 0.01 0.1 0.01
v 0 0.1 0.01
f 1 2 4
f 2 3 4
f 6 1 5
f 6 2 1
f 7 2 6
f 7 3 2
f 4 3 7
f 4 7 8
f 5 1 4
f 4 8 5
f 8 6 5
f 8 7 6
#end
)";
    Mesh m;
    stringstream inputstream, outputstream;
    inputstream << input;
    m.load(inputstream);
    m.save(outputstream);
    string outputstring;
    getline(outputstream, outputstring, '\0');
    ASSERT_STREQ(&(output[0]), &(outputstring[0]));
}

TEST(Mesh, BlenderTest) {
  const string input = R"(# Blender v2.71 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 1.000000 -1.000000 -1.000000
v 1.000000 -1.000000 1.000000
v -1.000000 -1.000000 1.000000
v -1.000000 -1.000000 -1.000000
v 1.000000 1.000000 -1.000000
v 1.000000 1.000000 1.000001
v -1.000000 1.000000 1.000000
v -1.000000 1.000000 -1.000000
vt 1.000000 0.333333
vt 1.000000 0.666667
vt 0.666667 0.666667
vt 0.666667 0.333333
vt 0.666667 0.000000
vt 0.333333 0.333333
vt 0.000000 0.333333
vt 0.000000 0.000000
vt 0.333333 0.000000
vt 0.000000 1.000000
vt 0.000000 0.666667
vt 0.333333 0.666667
vt 1.000000 0.000000
vt 0.333333 1.000000
usemtl Material
s off
f 2/1 3/2 4/3
f 8/1 7/4 6/5
f 1/6 5/7 6/8
f 2/6 6/9 7/5
f 7/10 8/11 4/12
f 1/6 4/12 8/11
f 1/4 2/1 4/3
f 5/13 8/1 6/5
f 2/9 1/6 6/8
f 3/4 2/6 7/5
f 3/14 7/10 4/12
f 5/7 1/6 8/11
)";
  const string output = R"(v 1 -1 -1
v 1 -1 1
v -1 -1 1
v -1 -1 -1
v 1 1 -1
v 1 1 1
v -1 1 1
v -1 1 -1
f 2 3 4
f 8 7 6
f 1 5 6
f 2 6 7
f 7 8 4
f 1 4 8
f 1 2 4
f 5 8 6
f 2 1 6
f 3 2 7
f 3 7 4
f 5 1 8
#end
)";
    Mesh m;
    stringstream inputstream, outputstream;
    inputstream << input;
    m.load(inputstream);
    m.save(outputstream);
    string outputstring;
    getline(outputstream, outputstring, '\0');
    ASSERT_STREQ(&(output[0]), &(outputstring[0]));

}

TEST(MeshVol, CubeTest) {
  const string input = R"(#v 12
#e 2
0 0 0
0 0 0.5
0 0.5 0
0 0.5 0.5
0 1 0
0 1 0.5
0.5 0 0
0.5 0 0.5
0.5 0.5 0
0.5 0.5 0.5
0.5 1 0
0.5 1 0.5
8 0 1 2 3 6 7 8 9
8 2 3 4 5 8 9 10 11

)";

  const string output = R"(#vertices 12
#elements 2
0 0 0
0 0 0.5
0 0.5 0
0 0.5 0.5
0 1 0
0 1 0.5
0.5 0 0
0.5 0 0.5
0.5 0.5 0
0.5 0.5 0.5
0.5 1 0
0.5 1 0.5
8 0 1 2 3 6 7 8 9
8 2 3 4 5 8 9 10 11
)";
    MeshVol m;
    stringstream inputstream, outputstream;
    inputstream << input;
    m.load(inputstream);
    m.save(outputstream);
    string outputstring;
    getline(outputstream, outputstring, '\0');
    ASSERT_STREQ(&(output[0]), &(outputstring[0]));
}

TEST(MeshVol, TetgenTest) {
  const string inputNode = R"(20  3  0  0
   0    0.01  0  0.01
   1    0  0  0.01
   2    0  0  0
   3    0  0.10000000000000001  0
   4    0.01  0.10000000000000001  0
   5    0.01  0  0
   6    0  0.10000000000000001  0.01
   7    0.01  0.10000000000000001  0.01
   8    0.01  0.050000000000000003  0.01
   9    0.01  0.050000000000000003  0
  10    0  0.050000000000000003  0.01
  11    0  0.050000000000000003  0
  12    0.01  0.075000000000000011  0.01
  13    0  0.075000000000000011  0
  14    0  0.075000000000000011  0.01
  15    0.01  0.075000000000000011  0
  16    0.01  0.025000000000000001  0.01
  17    0.01  0.025000000000000001  0
  18    0  0.025000000000000001  0.01
  19    0  0.025000000000000001  0
# Generated by ./tetgen -pqzV ../cubeb.stl 
)";

const string inputEle = R"(24  4  0
    0       0    17     2    19
    1       3    15     4    12
    2       8    15    13    12
    3       6     3     7    12
    4       8     9    11    13
    5       3    14    13    12
    6       3    14    12     6
    7      10     8    11    13
    8       8    14    13    10
    9       8    17    16    19
   10      10    19     8    18
   11       9    19     8    11
   12       7     3     4    12
   13       8    14    12    13
   14       3    15    12    13
   15       8    15     9    13
   16       9    19    17     8
   17      17     0    16    19
   18      18     8    16    19
   19       0    17     5     2
   20       1    19     0     2
   21       1    19    18     0
   22      10    19    11     8
   23       0    18    16    19
# Generated by ./tetgen -pqzV ../cubeb.stl 
)";

  MeshVol m;
  stringstream nodeStream,eleStream;
  nodeStream << inputNode;
  eleStream  << inputEle;
  m.loadTet(nodeStream, eleStream);
  ASSERT_EQ(0.01,  m.v[5][0]);
  ASSERT_EQ(0.025, m.v[19][1]);
  ASSERT_EQ(16,    m.e[23][2]);
  ASSERT_EQ(17,    m.e[0][1]);
  ASSERT_EQ(12,    m.e[2][3]);
}

TEST(MeshVol, TetgenEdgeTest) {
const string inputEdge = R"(24  1
    0      2     5  -1
    1     12     8  -1
    2      1     2  -1
    3      5     0  -1
    4      0     1  -1
    5      6    14  -1
    6      7    12  -1
    7      3     4  -1
    8      4    15  -1
    9      2    19  -1
   10      4     7  -1
   11      8    16  -1
   12     19    11  -1
   13     10    18  -1
   14     18     1  -1
   15      7     6  -1
   16      9    17  -1
   17     16     0  -1
   18      6     3  -1
   19     15     9  -1
   20     11    13  -1
   21     14    10  -1
   22     17     5  -1
   23     13     3  -1
# Generated by ./tetgen -pqzV ../cubeb.stl 
)";

  MeshVol m;
  stringstream edgeStream;
  edgeStream << inputEdge;
  m.loadTetEdge(edgeStream);

  ASSERT_EQ(5,     m.edges[0][1]);
  ASSERT_EQ(19,    m.edges[9][1]);
  ASSERT_EQ(13,    m.edges[23][0]);
  
}
