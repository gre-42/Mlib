#pragma once
#include <Mlib/Render/Ui/IList_View_Drawer.hpp>
#include <functional>
#include <sstream>
#include <string>

namespace Mlib {

class TextResource;
class ILayoutPixels;
class IEvaluatedWidget;
enum class ListViewOrientation;

class ListViewStringDrawer: public IListViewDrawer {
public:
    ListViewStringDrawer(
        ListViewOrientation orientation,
        TextResource& renderable_text,
        const ILayoutPixels& line_distance,
        const IEvaluatedWidget& ew,
        int height,
        float ydpi,
        const std::function<std::string(size_t)>& transformation);
    // IListViewDrawer
    virtual size_t max_entries_visible() const override;
    virtual void draw_left_dots() override;
    virtual void draw_right_dots(size_t filtered_index) override;
    virtual void draw_entry(
        size_t index,
        size_t filtered_index,
        bool is_selected,
        bool is_first) override;

    void render(int height, float ydpi);
private:
    std::string delimiter_;
    std::string sel_left_;
    std::string sel_right_;
    std::stringstream sstr_;
    ListViewOrientation orientation_;
    std::function<std::string(size_t)> transformation_;
    const ILayoutPixels& line_distance_;
    TextResource& renderable_text_;
    const IEvaluatedWidget& ew_;
    int height_;
    float ydpi_;
};

}