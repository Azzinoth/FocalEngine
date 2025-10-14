#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../FEngine.h"
using namespace FocalEngine;

namespace py = pybind11;

PYBIND11_MODULE(FocalEnginePython, m) {
    m.doc() = "Python bindings for Focal Engine";

    // FIX ME: All other functions are a little less susceptible to multithreading issues
    m.def("create_screenshot",
        [](const std::string& SceneID) -> pybind11::str {
			FEScene* Scene = SCENE_MANAGER.GetScene(SceneID);
			if (Scene == nullptr)
				throw py::value_error("Scene with ID '" + SceneID + "' does not exist");

            FETexture* Screenshot = RENDERER.CreateScreenshot(Scene);
			if (Screenshot == nullptr)
				throw py::value_error("Failed to create screenshot for scene with ID '" + SceneID + "'");

			std::string FilePath = RESOURCE_MANAGER.GetEngineFolder() + "/screenshot.png";
			RESOURCE_MANAGER.ExportFETextureToPNG(Screenshot, FilePath.c_str());
            return FilePath;
        },
        py::arg("SceneID"),
        py::return_value_policy::reference,
        "Create a screenshot of a scene and save it to PNG.\n"
        "Args:\n"
        "  SceneID: Scene ID.\n"
        "Returns: Absolute PNG file path.");

    py::class_<glm::vec3>(m, "Vector3")
        .def(py::init<float, float, float>())
        .def_readwrite("x", &glm::vec3::x)
        .def_readwrite("y", &glm::vec3::y)
        .def_readwrite("z", &glm::vec3::z);

    py::class_<FEEntity, std::unique_ptr<FEEntity, py::nodelete>>(m, "Entity")
        .def("GetID", &FEEntity::GetObjectID)
        .def("GetName", &FEEntity::GetName)
        .def("GetPosition",
            [](FEEntity& CurrentEntity) -> glm::vec3 {
                if (!CurrentEntity.HasComponent<FETransformComponent>())
                    throw py::value_error("Entity does not have a Transform component");

                return CurrentEntity.GetComponent<FETransformComponent>().GetPosition();
            },
            "Get the world position of the entity")
        .def("SetPosition",
            [](FEEntity& CurrentEntity, const glm::vec3& Position) {
                if (!CurrentEntity.HasComponent<FETransformComponent>())
                    throw py::value_error("Entity does not have a Transform component");

                CurrentEntity.GetComponent<FETransformComponent>().SetPosition(Position);
            },
            py::arg("Position"),
            "Set the world position of the entity");

    // Non-owning holder to avoid calling private destructor.
    py::class_<FEScene, std::unique_ptr<FEScene, py::nodelete>>(m, "Scene")
        // no .def(py::init<>()) !
        .def("get_ID", &FEScene::GetObjectID)
        .def("get_entity",
            [](FEScene& self, const std::string& id) -> FEEntity* {
                return self.GetEntity(id);
            },
            py::arg("id"),
            py::return_value_policy::reference)
        .def("get_entity_id_list", &FEScene::GetEntityIDList)
        .def("get_entity_by_name",
            [](FEScene& self, const std::string& name) {
                return self.GetEntityByName(name);  // std::vector<FEEntity*>
            },
            py::arg("name"),
            py::return_value_policy::reference)  // non-owning elements
        .def("create_entity",
            [](FEScene& self, const std::string& name, const std::string& force_id) {
                return self.CreateEntity(name, force_id); // FEEntity*
            },
            py::arg("name") = "", py::arg("force_id") = "",
            py::return_value_policy::reference);

    py::class_<FESceneManager,
        std::unique_ptr<FESceneManager, py::nodelete>>(m, "SceneManager")
        // no .def(py::init<>()) because destructor is private (SINGLETON_PRIVATE_PART)
        .def_property_readonly_static(
            "instance",
            [](py::object) -> FESceneManager& { return FESceneManager::GetInstance(); },
            py::return_value_policy::reference,
            "Get the global SceneManager.")
        .def("get_scene",
            [](FESceneManager& self, pybind11::str pyid) -> FEScene* {
                std::string id = pyid.cast<std::string>();
                if (id.empty())
                    throw pybind11::value_error("Scene id cannot be empty");

                return self.GetScene(id);
            },
            py::arg("id"),
            py::return_value_policy::reference)
        .def("get_starting_scene",
            [](FESceneManager& self) -> FEScene* {
                return self.GetStartingScene();
            },
		    py::return_value_policy::reference);
}