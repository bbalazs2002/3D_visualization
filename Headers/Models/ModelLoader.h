#pragma once

#include "../include_all.h"

class ModelLoader {
public:
    static inline std::filesystem::path ResolveMTLSearchPath(const std::filesystem::path& objPath, const std::filesystem::path& mtlSearchPath)
    {
        // Ha az mtlSearchPath abszolút, nincs teendõ
        if (mtlSearchPath.is_absolute())
            return mtlSearchPath;

        // Ellenkezõ esetben az objPath mappájától számítjuk
        std::filesystem::path objDir = objPath.parent_path();

        // Normalizálás: összeillesztjük és leellenõrizzük, hogy létezik-e
        std::filesystem::path resolved = std::filesystem::weakly_canonical(objDir / mtlSearchPath);

        return resolved;
    }
    static inline std::filesystem::path ResolveTexturePath(const std::string& texPath, const std::filesystem::path& mtlSearchPath) {
        // normal texture path
        std::string pathName = texPath;
        std::replace(pathName.begin(), pathName.end(), '/', '\\');
        std::filesystem::path path = pathName;
        if (!path.is_absolute()) {
            path = mtlSearchPath / pathName;
        }
        return path;
    }
    static ModelLoaderReturn LoadFromOBJ(const std::filesystem::path& objPath,
        const std::filesystem::path& mtlSearchPath = "./") {

        std::filesystem::path absMtlSearchPath = ModelLoader::ResolveMTLSearchPath(objPath, mtlSearchPath);

        Log::logToConsole("Reading .obj file: ", objPath);
        Log::logToConsole(".mtl path: ", absMtlSearchPath);

        ModelLoaderReturn retVal;

        tinyobj::ObjReaderConfig config;
        config.mtl_search_path = absMtlSearchPath.string();

        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(objPath.string(), config)) {
            Log::errorToConsole("TinyObjLoader error: ", reader.Error());
            throw std::runtime_error("Failed to load OBJ");
        }

        const auto& attrib = reader.GetAttrib();
        const auto& shapes = reader.GetShapes();
        const auto& materials = reader.GetMaterials();

        // Anyagok betöltése SDL textúrákkal
        Log::logToConsole("Materials found (", materials.size(), ")");
        // Set default material
        if (materials.size() <= 0) {
            auto material = new Material();
            retVal.materials.push_back(material);
        }
        for (const auto& mat : materials) {
            auto material = new Material();
            material->name = mat.name;
            material->diffuseColor = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
            material->specularColor = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
            material->ambientColor = glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
            material->shininess = mat.shininess;

            if (!mat.diffuse_texname.empty())
                material->diffuseTex = Material::LoadTexture(ResolveTexturePath(mat.diffuse_texname, absMtlSearchPath));
            if (!mat.specular_texname.empty())
                material->specularTex = Material::LoadTexture(ResolveTexturePath(mat.specular_texname, absMtlSearchPath));
            if (!mat.emissive_texname.empty())
                material->emissionTex = Material::LoadTexture(ResolveTexturePath(mat.emissive_texname, absMtlSearchPath));
            if (!mat.normal_texname.empty())
                material->normalTex = Material::LoadTexture(ResolveTexturePath(mat.normal_texname, absMtlSearchPath));

            retVal.materials.push_back(material);
        }

        // Mesh-ek létrehozása
        Log::logToConsole("Shapes found (", shapes.size(), ")");

        for (const auto& shape : shapes) {
            // Log::logToConsole("Processing shape: ", shape.name);

            // --- Elõször hozzunk létre egy map-et, ami a material_id -> (verteces, indices) párokat tárolja
            std::unordered_map<int, std::vector<Vertex>> verticesPerMaterial;
            std::unordered_map<int, std::vector<GLuint>> indicesPerMaterial;

            size_t index_offset = 0;

            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                int fv = shape.mesh.num_face_vertices[f];
                int mat_id = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[f];

                // ha nincs anyag megadva, alapértelmezett legyen 0
                if (mat_id < 0 || mat_id >= (int)retVal.materials.size()) {
                    mat_id = 0;
                }

                for (size_t v = 0; v < fv; v++) {
                    Vertex vert{};
                    tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                    // --- pozíció
                    vert.position = glm::vec3(
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]
                    );

                    // --- normál
                    if (idx.normal_index >= 0) {
                        vert.normal = glm::vec3(
                            attrib.normals[3 * idx.normal_index + 0],
                            attrib.normals[3 * idx.normal_index + 1],
                            attrib.normals[3 * idx.normal_index + 2]
                        );
                    }

                    // --- textúra koordináta
                    if (idx.texcoord_index >= 0) {
                        vert.texcoord = glm::vec2(
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            attrib.texcoords[2 * idx.texcoord_index + 1]
                        );
                    }

                    // --- hozzáadjuk az adott anyaghoz tartozó listához
                    auto& vList = verticesPerMaterial[mat_id];
                    auto& iList = indicesPerMaterial[mat_id];
                    vList.push_back(vert);
                    iList.push_back(static_cast<GLuint>(iList.size()));
                }

                index_offset += fv;
            }

            // Minden anyaghoz külön Mesh példányt hozunk létre
            for (auto& [mat_id, verts] : verticesPerMaterial) {
                auto& inds = indicesPerMaterial[mat_id];

                auto mesh = new Mesh();
                mesh->SetMaterial(retVal.materials[mat_id]);
                mesh->Build(std::move(verts), std::move(inds));
                retVal.meshes.push_back(mesh);

                // Log::logToConsole("Submesh built (material ", mat_id, ", verts: ", verts.size(), ")");
            }
        }


        return retVal;
    }
};