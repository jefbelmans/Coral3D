#ifndef CORAL_INPUT_H
#define CORAL_INPUT_H

#include <map>
#include <vector>
#include <functional>

// https://stackoverflow.com/questions/55573238/how-do-i-do-a-proper-input-class-in-glfw-for-a-game-engine
class GLFWwindow;
namespace coral_3d
{
    class coral_input final
    {
    public:
        using Callback = std::pair<bool,std::function<void()>>;
        coral_input();
        ~coral_input();

        void add_callback(int key, const Callback& callback);
        static void initialize(GLFWwindow* ptr_window);

    private:
        void update_key_state(int key, int state);

        static void callback(GLFWwindow* ptr_window, int key, int scancode, int action, int mods);
        static std::vector<coral_input*> instances_;

        std::map<int, std::vector<Callback>> callbacks_;
    };
} // coral_3d

#endif // CORAL_INPUT_H
