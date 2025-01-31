#pragma once

#include <vector>

#include "gui_element_types.hpp"
#include "triggers.hpp"

namespace ui {

class decision_requirements : public image_element_base {};
class make_decision : public  button_element_base {};

// -------------
// Decision Name
// -------------

class decision_name : public simple_text_element_base {
public:
  void on_update(sys::state& state) noexcept override {
    Cyto::Any payload = dcon::decision_id{};
    if(parent) {
      parent->impl_get(state, payload);
      auto id = any_cast<dcon::decision_id>(payload);
      auto fat_id = dcon::fatten(state.world, id);
      auto name = text::produce_simple_string(state, fat_id.get_name());
      set_text(state, name);
    }
  }
};

// --------------
// Decision Image
// --------------

class decision_image : public image_element_base {
public:
  void on_update(sys::state& state) noexcept override {
    Cyto::Any payload = dcon::decision_id{};
    if(parent) {
      parent->impl_get(state, payload);
      auto id = any_cast<dcon::decision_id>(payload);
      auto fat_id = dcon::fatten(state.world, id);
      base_data.data.image.gfx_object = fat_id.get_image();
    }
  }
};

// --------------------
// Decision Description
// --------------------

class decision_desc : public multiline_text_element_base {
private:
  dcon::text_sequence_id description;
  void populate_layout(sys::state& state, text::endless_layout& contents) noexcept {
    auto box = text::open_layout_box(contents);
    text::add_to_layout_box(contents, state, box, description, text::substitution_map{ });
    text::close_layout_box(contents, box);
  }

public:
  void on_update(sys::state& state) noexcept override {
    Cyto::Any payload = dcon::decision_id{};
    if(parent) {
      parent->impl_get(state, payload);
      auto id = any_cast<dcon::decision_id>(payload);
      auto fat_id = dcon::fatten(state.world, id);
      description = fat_id.get_description();
    }
    int16_t X_FUDGE_FACTOR = 50; // TODO: why is this necessary? Without it there is horizontal clipping
    auto container = text::create_endless_layout(
      internal_layout,
      text::layout_parameters{ 0, 0, static_cast<int16_t>(base_data.size.x - X_FUDGE_FACTOR), static_cast<int16_t>(base_data.size.y), base_data.data.text.font_handle, 0, text::alignment::left, text::text_color::black }
    );
    populate_layout(state, container);
  }

  message_result test_mouse(sys::state& state, int32_t x, int32_t y) noexcept override {
    return message_result::unseen;
  }
};

// ---------------
// Ignore Checkbox
// ---------------

class ignore_checkbox : public checkbox_button {
private:
  bool checked{false};

public:
  void button_action(sys::state& state) noexcept override {
    checked = !checked;
  }

  bool is_active(sys::state& state) noexcept override {
    return checked;
  }
};

// -------------
// Decision Item
// -------------

class decision_item : public listbox_row_element_base<dcon::decision_id> {
public:
  std::unique_ptr<ui::element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
    if(name == "decision_name") {
      return make_element_by_type<decision_name>(state, id);
    } else if (name == "decision_image"){
      return make_element_by_type<decision_image>(state, id);
    } else if (name == "decision_desc") {
      return make_element_by_type<decision_desc>(state, id);
    } else if (name == "requirements") {
      return make_element_by_type<decision_requirements>(state, id);
    } else if (name == "ignore_checkbox") {
      return make_element_by_type<ignore_checkbox>(state, id);
    } else if (name == "make_decision") {
      return make_element_by_type<make_decision>(state, id);
    } else {
      return nullptr;
    }
  }

  message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
    if(payload.holds_type<dcon::decision_id>()) {
      payload.emplace<dcon::decision_id>(content);
      return message_result::consumed;
    } else if (payload.holds_type<wrapped_listbox_row_content<dcon::decision_id>>()) {
      return listbox_row_element_base<dcon::decision_id>::get(state, payload);
    } else {
      return message_result::unseen;
    }
  }

  void update(sys::state& state) noexcept override { 
    for(auto& child : children){
      child->impl_on_update(state);
    }
  }
};

// ----------------
// Decision Listbox
// ----------------

class decision_listbox : public listbox_element_base<decision_item, dcon::decision_id> {
protected:
  std::string_view get_row_element_name() {
    return "decision_entry";
  }
};

// ----------------
// Decision Window
// ----------------

class decision_window : public window_element_base {
private:
  decision_listbox* decision_list{nullptr};

  std::vector<dcon::decision_id> get_decisions(sys::state& state) { 
    std::vector<dcon::decision_id> list;
    auto n = state.local_player_nation;
    for(uint32_t i = state.world.decision_size(); i-- > 0; ) {
      dcon::decision_id did{ dcon::decision_id::value_base_t(i) };
      auto lim = state.world.decision_get_potential(did); 
      if(!lim || trigger::evaluate_trigger(state, lim, trigger::to_generic(n), trigger::to_generic(n), 0)) { 
        list.push_back(did);
      }
    }
    return list;
  }

public:
  void on_create(sys::state& state) noexcept override {
    window_element_base::on_create(state);
    set_visible(state, false);
    
    decision_list->row_contents = get_decisions(state);
    decision_list->update(state);
  }

  void on_update(sys::state& state) noexcept override {
    decision_list->row_contents = get_decisions(state);
    decision_list->update(state);
  }

  std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
    if(name == "decision_listbox") {
      auto ptr = make_element_by_type<decision_listbox>(state, id);
      decision_list = ptr.get();
      return ptr;
    } else {
      return nullptr;
    }
  }
};

} //namespace
