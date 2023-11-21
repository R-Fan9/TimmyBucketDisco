#ifndef OBJ_H
#define OBJ_H

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_DOUBLE
#include <tiny_obj_loader.h>

class Obj
{
public:
  Obj(const std::string &file_path) : obj_path(file_path)
  {
    std::string warn, err;

    bool bTriangulate = true;
    bool bSuc = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                 obj_path.c_str(), nullptr, bTriangulate);

    if (!bSuc)
    {
      throw std::runtime_error("tinyobj error:" + err);
    }
  }

  std::vector<tinyobj::shape_t> getShapes()
  {
    return shapes;
  }

  std::vector<tinyobj::real_t> getVertices()
  {
    return attrib.vertices;
  }

  std::vector<tinyobj::real_t> getNormals()
  {
    return attrib.normals;
  }

  std::vector<tinyobj::real_t> getTexCoords()
  {
    return attrib.texcoords;
  }

private:
  std::string obj_path;
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
};

#endif // !OBJ_H
