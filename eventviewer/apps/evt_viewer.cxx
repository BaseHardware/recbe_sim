#include "eventviewer/EventViewer.h"

#include "TApplication.h"
#include "TGClient.h"

int main(int argc, char **argv) {
    const char *filename = argc > 1 ? argv[1] : "bl10sim/simout.root";
    TApplication app("evt_viewer", &argc, argv);

    new eventviewer::EventViewer(gClient->GetRoot(), filename);
    app.Run();
    return 0;
}
