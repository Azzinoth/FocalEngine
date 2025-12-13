#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../FEngine.h"
using namespace FocalEngine;

namespace py = pybind11;

class ScreenshotFlag {
private:
    std::atomic<bool> flag{ false };

public:
    void set(bool value) {
        flag.store(value, std::memory_order_release);
    }

    bool get() const {
        return flag.load(std::memory_order_acquire);
    }

    void wait_until_clear() {
        // Busy wait until flag is false
        while (flag.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    }
};

PYBIND11_MODULE(FocalEnginePython, m) {
    m.doc() = "Python bindings for Focal Engine";

    m.def("get_render_flag",
        []() -> bool {
            return RENDERER.GetScreenshotFlag().load(std::memory_order_acquire);
        },
        "Check if screenshot is being requested");

    m.def("set_render_flag",
        [](bool value) {
            RENDERER.GetScreenshotFlag().store(value, std::memory_order_release);
        },
        py::arg("value"),
        "Set screenshot request flag");

    m.def("wait_for_screenshot",
        []() {
            // Release GIL while waiting so C++ thread can run
            py::gil_scoped_release release;

            // Wait until flag is set back to false by C++
            while (RENDERER.GetScreenshotFlag().load(std::memory_order_acquire)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        },
        "Wait until screenshot is complete (blocks until C++ clears the flag)");

    py::class_<glm::vec3>(m, "Vector3")
        .def(py::init<float, float, float>())
        .def_readwrite("x", &glm::vec3::x)
        .def_readwrite("y", &glm::vec3::y)
        .def_readwrite("z", &glm::vec3::z);

    py::class_<FEEntity, std::unique_ptr<FEEntity, py::nodelete>>(m, "Entity")
        .def("GetID", &FEEntity::GetObjectID)
        .def("GetName", &FEEntity::GetName)
        .def("SetName", &FEEntity::SetName, py::arg("Name"))
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
            "Set the world position of the entity")

        .def("GetRotation",
            [](FEEntity& CurrentEntity) -> glm::vec3 {
                if (!CurrentEntity.HasComponent<FETransformComponent>())
                    throw py::value_error("Entity does not have a Transform component");
                return CurrentEntity.GetComponent<FETransformComponent>().GetRotation();
            },
            "Get the world rotation of the entity (Euler angles in degrees)")

        .def("SetRotation",
            [](FEEntity& CurrentEntity, const glm::vec3& Rotation) {
                if (!CurrentEntity.HasComponent<FETransformComponent>())
                    throw py::value_error("Entity does not have a Transform component");
                CurrentEntity.GetComponent<FETransformComponent>().SetRotation(Rotation);
            },
            py::arg("Rotation"),
            "Set the world rotation of the entity (Euler angles in degrees)")

        .def("GetScale",
            [](FEEntity& CurrentEntity) -> glm::vec3 {
                if (!CurrentEntity.HasComponent<FETransformComponent>())
                    throw py::value_error("Entity does not have a Transform component");
                return CurrentEntity.GetComponent<FETransformComponent>().GetScale();
            },
            "Get the world scale of the entity")

        .def("SetScale",
            [](FEEntity& CurrentEntity, const glm::vec3& Scale) {
                if (!CurrentEntity.HasComponent<FETransformComponent>())
                    throw py::value_error("Entity does not have a Transform component");
                CurrentEntity.GetComponent<FETransformComponent>().SetScale(Scale);
            },
            py::arg("Scale"),
            "Set the world scale of the entity")

        .def("GetParentEntity",
            [](FEEntity& CurrentEntity) -> FEEntity* {
                return CurrentEntity.GetParentEntity();
            },
            py::return_value_policy::reference)

        .def("GetChildEntities",
            [](FEEntity& CurrentEntity) -> std::vector<FEEntity*> {
                return CurrentEntity.GetChildEntities();
            },
            py::return_value_policy::reference)

        .def("AttachTo",
            [](FEEntity& CurrentEntity, FEEntity* Parent, bool bPreserveWorldTransform) -> bool {
                return CurrentEntity.AttachTo(Parent, bPreserveWorldTransform);
            },
            py::arg("Parent"), py::arg("bPreserveWorldTransform") = true,
            "Attach this entity to a new parent entity")

        .def("Detach",
            [](FEEntity& CurrentEntity, bool bPreserveWorldTransform) -> bool {
                return CurrentEntity.Detach(bPreserveWorldTransform);
            },
            py::arg("bPreserveWorldTransform") = true,
            "Detach this entity from its parent entity")

        .def("IsChildOf",
            [](FEEntity& CurrentEntity, FEEntity* PotentialParent) -> bool {
                return CurrentEntity.IsChildOf(PotentialParent);
            },
            py::arg("PotentialParent"),
            "Check if this entity is a child of the given potential parent entity")
        .def("GetInstanceSeed",
            [](FEEntity& CurrentEntity) -> uint32_t {
                if (!CurrentEntity.HasComponent<FEInstancedComponent>())
                    throw py::value_error("Entity does not have an Instance component");
                return CurrentEntity.GetComponent<FEInstancedComponent>().SpawnInfo.Seed;
            },
            "Get the instance seed of the entity")
        .def("SetInstanceSeed",
            [](FEEntity& CurrentEntity, uint32_t Seed) {
                if (!CurrentEntity.HasComponent<FEInstancedComponent>())
                    throw py::value_error("Entity does not have an Instance component");
                CurrentEntity.GetComponent<FEInstancedComponent>().SpawnInfo.Seed = Seed;
				// demonstration purposes only: respawn immediately
                INSTANCED_RENDERING_SYSTEM.ClearInstance(&CurrentEntity);
                INSTANCED_RENDERING_SYSTEM.PopulateInstance(&CurrentEntity, CurrentEntity.GetComponent<FEInstancedComponent>().SpawnInfo);
            },
            py::arg("Seed"),
				"Set the instance seed of the entity")
		.def("GetInstanceCount",
            [](FEEntity& CurrentEntity) -> uint32_t {
                if (!CurrentEntity.HasComponent<FEInstancedComponent>())
                    throw py::value_error("Entity does not have an Instance component");
                return CurrentEntity.GetComponent<FEInstancedComponent>().GetInstanceCount();
            },
            "Get the instance count of the entity")
        .def("SetInstanceCount",
            [](FEEntity& CurrentEntity, uint32_t Count) {
                if (!CurrentEntity.HasComponent<FEInstancedComponent>())
                    throw py::value_error("Entity does not have an Instance component");
                CurrentEntity.GetComponent<FEInstancedComponent>().SpawnInfo.ObjectCount = Count;
                // demonstration purposes only: respawn immediately
                INSTANCED_RENDERING_SYSTEM.ClearInstance(&CurrentEntity);
                INSTANCED_RENDERING_SYSTEM.PopulateInstance(&CurrentEntity, CurrentEntity.GetComponent<FEInstancedComponent>().SpawnInfo);
			},
            py::arg("Count"),
			"Set the instance count of the entity");

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