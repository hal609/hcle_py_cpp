// src/hcle/python/python_interface.cpp

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "hcle/environment/hcle_environment.hpp"
#include "hcle/common/exceptions.hpp"

namespace py = pybind11;

void init_vector_bindings(py::module_ &m);

// The function signature for vector_to_numpy needs to be corrected as well
// to avoid the 'unreferenced parameter' warning and to be more efficient.
// This version takes a const reference.
py::array_t<uint8_t> vector_to_numpy(const std::vector<uint8_t> &vec)
{
    // py::capsule free_when_done(vec.data(), [](void* f) { /* No-op for const& */ });
    return py::array_t<uint8_t>(
        {static_cast<py::ssize_t>(vec.size())}, // Shape
        {sizeof(uint8_t)},                      // Strides
        vec.data()                              // Pointer
    );
}

PYBIND11_MODULE(_hcle_py, m)
{
    // Use the fully qualified name: hcle::environment::HCLEnvironment
    py::class_<hcle::environment::HCLEnvironment>(m, "HCLEnvironment")
        .def(py::init<std::string, std::string, std::string>(),
             py::arg("rom_path"), py::arg("game_name"), py::arg("render_mode"))
        // Use the ::& operator to get the address of the member function
        .def("act", &hcle::environment::HCLEnvironment::act, "Performs an action and returns the reward")
        .def("reset", &hcle::environment::HCLEnvironment::reset, "Resets the environment")
        .def("is_done", &hcle::environment::HCLEnvironment::isDone, "Checks if the episode is terminated")
        .def("get_action_set", [](hcle::environment::HCLEnvironment &env)
             { return env.getActionSet(); })
        .def("get_ram", [](hcle::environment::HCLEnvironment &env)
             { return vector_to_numpy(env.getRAM()); })
        .def("get_screen_rgb", [](hcle::environment::HCLEnvironment &self)
             {
            // Create a NumPy array to hold the screen data
            auto obs = py::array_t<uint8_t>({240, 256, 3});
            // Call the C++ function, passing a pointer to the array's buffer
            self.getScreenRGB(obs.mutable_data());
            // Return the populated NumPy array
            return obs; });
    init_vector_bindings(m);
    py::register_exception<hcle::common::WindowClosedException>(m, "WindowClosedException");
}