#pragma once
#include "dsc_ui.h"
#include "ui_enum.h"
#include "ui_coord.h"
#include "vector_ui_coord2.h"
#include <dsc_common\vector_float4.h>

namespace DscCommon
{
	class FileSystem;
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
	template <typename TYPE>
	class Vector4;
	typedef Vector4<float> VectorFloat4;
}

namespace DscDag
{
	class DagCollection;
	class IDagNode;
	typedef IDagNode* NodeToken;
}

namespace DscRender
{
	class DrawSystem;
	class IRenderTarget;
}

namespace DscRenderResource
{
	class Frame;
	class GeometryGeneric;
	class RenderTargetPool;
	class RenderTargetTexture;
	class Shader;
	class ShaderConstantBuffer;
	class ShaderResource;
}

namespace DscDagRender
{
	class DagResource;
}

namespace DscText
{
	class TextManager;
	class TextRun;
}

namespace DscUi
{
	constexpr int32 _multi_gradient_fill_array_size = 8;

	struct ComponentConstructionHelper
	{
		TUiComponentType _component_type;
		DscCommon::VectorFloat4 _clear_colour;

		bool _has_scroll = false;
		bool _has_manual_scroll_x = false;
		bool _has_manual_scroll_y = false;

		bool _has_fill = false;
		DscCommon::VectorFloat4 _fill;

		std::shared_ptr<DscRenderResource::ShaderResource> _texture;

		// we could use move semantics to have the text run as a unique_ptr, but kind of throwing around the ComponentConstructionHelper by copy
		std::shared_ptr<DscText::TextRun> _text_run = {};
		DscDag::NodeToken _text_run_node = nullptr;
		DscText::TextManager* _text_manager = nullptr;

		// possibly more than just the stack component will use this
		TUiFlow _flow_direction = TUiFlow::TCount;

		bool _has_ui_scale_by_avaliable_width = false;
		int32 _scale_width_low_threashhold = 0; // example 800
		float _scale_factor = 0.0f; // 0.0015789 for scale of 4.8 when width is 3040 more than 800,

		bool _has_child_slot_data = false;
		VectorUiCoord2 _child_size = {};
		VectorUiCoord2 _child_pivot = {};
		VectorUiCoord2 _attach_point = {};

		bool _has_desired_size = false;
		VectorUiCoord2 _desired_size = {};

		bool _has_padding = false;
		UiCoord _padding_left = {};
		UiCoord _padding_top = {};
		UiCoord _padding_right = {};
		UiCoord _padding_bottom = {};

		// gap between items for stack
		bool _has_gap = false;
		UiCoord _gap = {};

		bool _has_gradient_fill = false;
		TGradientFillConstantBuffer _gradient_fill_constant_buffer = {};

		bool _has_multi_gradient_fill = false;
		//TGradientFillConstantBuffer _multi_gradient_fill_constant_buffer[_multi_gradient_fill_array_size] = {};
		DscDag::NodeToken _multi_gradient_fill_node = {}; // type std::vector<TGradientFillConstantBuffer>
		// move towards passing a node token, rather than a std::string to fetch a named node from the DagCollection? (for large chunks of data)

		DscDag::NodeToken _crossfade_active_child = {};
		bool _has_crossfade_child_amount = false;
		float _crossfade_child_amount = 0.0f;

		bool _has_child_stack_data = false; // size data for child of stack
		UiCoord _stack_size = {};
		UiCoord _stack_pivot = {};
		UiCoord _stack_parent_attach_point = {};

		bool _desired_size_from_children_max = false; // otherwise from text? or matches avaliable

		bool _has_input = false;
		std::function<void(DscDag::NodeToken)> _input_click_callback = {};
		bool _has_input_rollover_accumulate = false;
		bool _has_input_active_touch_pos = false;

		bool _has_parent_index = false;
		int32 _parent_index = 0; // what happens to the existing child if you overwrite a child

		int32 _celtic_knot_size_pixels = 0;
		DscCommon::VectorFloat4 _celtic_knot_tint = {};

		bool _has_effect_scale = false;
		float _effect_strength = 0.0f;

	public:
		ComponentConstructionHelper& SetClearColour(
			const DscCommon::VectorFloat4& in_clear_colour
		)
		{
			_clear_colour = in_clear_colour;
			return *this;
		}
		ComponentConstructionHelper& SetChildSlot(
			const VectorUiCoord2& in_child_size = VectorUiCoord2(UiCoord(0, 1.0f), UiCoord(0, 1.0f)),
			const VectorUiCoord2& in_child_pivot = VectorUiCoord2(UiCoord(0, 0.0f), UiCoord(0, 0.0f)),
			const VectorUiCoord2& in_attach_point = VectorUiCoord2(UiCoord(0, 0.0f), UiCoord(0, 0.0f))
			)
		{
			_has_child_slot_data = true;
			_child_size = in_child_size;
			_child_pivot = in_child_pivot;
			_attach_point = in_attach_point;
			return *this;
		}

		ComponentConstructionHelper& SetDesiredSize(
			const VectorUiCoord2& in_desired_size
		)
		{
			_has_desired_size = true;
			_desired_size = in_desired_size;
			return *this;
		}
		ComponentConstructionHelper& SetPadding(
			const UiCoord& in_padding_left,
			const UiCoord& in_padding_top,
			const UiCoord& in_padding_right,
			const UiCoord& in_padding_bottom
			)
		{
			_has_padding = true;
			_padding_left = in_padding_left;
			_padding_top = in_padding_top;
			_padding_right = in_padding_right;
			_padding_bottom = in_padding_bottom;
			return *this;
		}

		ComponentConstructionHelper& SetUiScaleByWidth(
			const int32 in_scale_width_low_threashhold,
			const float in_scale_factor
		)
		{
			_has_ui_scale_by_avaliable_width = true;
			_scale_width_low_threashhold = in_scale_width_low_threashhold;
			_scale_factor = in_scale_factor;
			return *this;
		}

		ComponentConstructionHelper& SetChildStackData(
			const UiCoord& in_stack_size,
			const UiCoord& in_stack_pivot,
			const UiCoord& in_stack_parent_attach_point
		)
		{
			_has_child_stack_data = true;
			_stack_size = in_stack_size;
			_stack_pivot = in_stack_pivot;
			_stack_parent_attach_point = in_stack_parent_attach_point;
			return *this;
		}

		ComponentConstructionHelper& SetInputData(
			const std::function<void(DscDag::NodeToken)>& in_click_callback_or_none = {},
			const bool in_has_input_rollover_accumulate = false,
			const bool in_has_input_active_touch_pos = false
		)
		{
			_has_input = true;
			_input_click_callback = in_click_callback_or_none;
			_has_input_rollover_accumulate = in_has_input_rollover_accumulate;
			_has_input_active_touch_pos = in_has_input_active_touch_pos;
			return *this;
		}

		ComponentConstructionHelper& SetParentIndex(
			const int32 in_parent_index
		)
		{
			_has_parent_index = true;
			_parent_index = in_parent_index;
			return *this;
		}

		ComponentConstructionHelper& SetCrossfadeChildAmount(
			const float in_crossfade_child_amount
		)
		{
			_has_crossfade_child_amount = true;
			_crossfade_child_amount = in_crossfade_child_amount;
			return *this;
		}

		ComponentConstructionHelper& SetHasEffectScale(
			const float in_effect_strength = 0.0f
		)
		{
			_has_effect_scale = true;
			_effect_strength = in_effect_strength;
			return *this;
		}
	};

	// is passing tick and in_self overkill for the crossfade, should this be moved out~ but want the crossfade active child node
	DscDag::NodeToken MakeComponentResourceGroup(
		DscDag::DagCollection& in_dag_collection,
		const ComponentConstructionHelper& in_construction_helper,
		DscDag::NodeToken in_time_delta,
		DscDag::NodeToken in_ui_scale,
		DscDag::NodeToken in_parent,
		DscDag::NodeToken in_self
	);

	ComponentConstructionHelper MakeComponentDebugGrid();
	ComponentConstructionHelper MakeComponentFill(const DscCommon::VectorFloat4& in_colour);
	ComponentConstructionHelper MakeComponentGradientFill(
		const TGradientFillConstantBuffer& in_gradient_fill
	);
	ComponentConstructionHelper MakeComponentMultiGradientFill(
		DscDag::NodeToken in_multi_gradient_fill_node
	);
	ComponentConstructionHelper MakeComponentImage(const std::shared_ptr<DscRenderResource::ShaderResource>& in_texture);
	ComponentConstructionHelper MakeComponentCanvas();
	ComponentConstructionHelper MakeComponentText(
		const std::shared_ptr<DscText::TextRun>& in_text_run,
		DscText::TextManager* const in_text_manager, // so, either the text manager needs to be told to upload the glyph texture before draw and we can grab the text shader pointer, or our draw method needs a ref to the text manager
		const bool in_has_scroll = true
		);
	ComponentConstructionHelper MakeComponentTextNode(
		DscDag::NodeToken in_text_run_node,
		DscText::TextManager* const in_text_manager, // so, either the text manager needs to be told to upload the glyph texture before draw and we can grab the text shader pointer, or our draw method needs a ref to the text manager
		const bool in_has_scroll = true
	);
	ComponentConstructionHelper MakeComponentStack(
		const TUiFlow in_flow_direction,
		const UiCoord& in_gap,
		const bool in_desired_size_from_children_max = true,
		const bool in_has_scroll = true
		);
	ComponentConstructionHelper MakeComponentCrossfade(
		DscDag::NodeToken in_crossfade_active_child
		);
	ComponentConstructionHelper MakeComponentCelticKnot(
		const int32 in_knot_size_pixels,
		const DscCommon::VectorFloat4& in_knot_tint = DscCommon::VectorFloat4(1.0f, 1.0f, 1.0f, 1.0f)
	);
}
