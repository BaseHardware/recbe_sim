#ifndef __eventviewer_EventViewerWidgets_h__
#define __eventviewer_EventViewerWidgets_h__

class TGCheckButton;
class TGColorSelect;
class TGComboBox;
class TGLabel;
class TGListTree;
class TGNumberEntry;
class TGTextView;

namespace eventviewer {
    class EventViewerWidgets {
      public:
        TGNumberEntry *eventEntry            = nullptr;
        TGLabel *fileLabel                   = nullptr;
        TGLabel *summaryLabel                = nullptr;
        TGCheckButton *showGeometry          = nullptr;
        TGCheckButton *showTracks            = nullptr;
        TGCheckButton *showSteps             = nullptr;
        TGTextView *metadataText             = nullptr;
        TGListTree *geometryTree             = nullptr;
        TGTextView *trackText                = nullptr;
        TGTextView *stepText                 = nullptr;
        TGComboBox *graphicalVerbosityBox    = nullptr;
        TGNumberEntry *viewXEntry            = nullptr;
        TGNumberEntry *viewYEntry            = nullptr;
        TGNumberEntry *viewZEntry            = nullptr;
        TGNumberEntry *upXEntry              = nullptr;
        TGNumberEntry *upYEntry              = nullptr;
        TGNumberEntry *upZEntry              = nullptr;
        TGColorSelect *backgroundColorSelect = nullptr;
    };
} // namespace eventviewer

#endif
