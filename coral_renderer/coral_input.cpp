#include "coral_input.h"
#include "coral_window.h"

#include <iostream>
#include <algorithm>

using namespace coral_3d;

std::vector<coral_input*> coral_input::instances_;

coral_input::coral_input()
{
    // Register this class
    coral_input::instances_.emplace_back(this);
}

coral_input::~coral_input()
{
    // Unregister this class
    instances_.erase(std::remove(instances_.begin(), instances_.end(), this),
                     instances_.end());
}

void coral_input::initialize(GLFWwindow* ptr_window)
{
    glfwSetKeyCallback(ptr_window, coral_input::callback);
}

void coral_input::update_key_state(int key, int state)
{
    for (auto& key_pair : callbacks_)
    {
        // If pressed key has registered callbacks
        if(key_pair.first == key)
        {
            // Loop through registered callbacks
            for(auto& callback : key_pair.second)
            {
                // If current key state matches the callback state, invoke
                // the callback
                if(callback.first == state)
                {
                    callback.second();
                }
            }
        }
    }
}

void coral_input::callback(GLFWwindow*, int key, int, int action, int)
{
    for(coral_input* input : instances_)
    {
        input->update_key_state(key, action);
    }
}

void coral_input::add_callback(int key, const coral_input::Callback& callback)
{
    callbacks_[key].emplace_back(callback);
}