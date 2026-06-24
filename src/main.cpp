#include "core/App.h"

int main() {
    App app;

    if (!app.init(1280, 720, "Robot 3D Roaming World")) {
        return 1;
    }

    app.run();
    app.shutdown();
    return 0;
}
