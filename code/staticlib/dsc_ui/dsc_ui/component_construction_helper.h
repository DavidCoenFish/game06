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

		bool _has_child_stack_data = false; // size data for child of stack
		UiCoord _stack_size = {};
		UiCoord _stack_pivot = {};
		UiCoord _stack_parent_attach_point = {};

		bool _desired_size_from_children_max = false; // otherwise from text? or matches avaliable

		bool _has_input = false;
		std::function<void(const UiComponentResourceNodeGroup&)> _input_click_callback = {};
		bool _has_input_rollover_accumulate = false;
		bool _has_for_input_flag = false;
		TUiInputStateFlag _for_input_state_flag = TUiInputStateFlag::TNone;

		bool _has_parent_index = false;
		int32 _parent_index = 0; // what happens to the existing child if you overwrite a child

		ComponentConstructionHelper& SetClearColour(
			const DscCommon::VectorFloat4& in_clear_colour
		)
		{
			_clear_colour = in_clear_colour;
			return *this;
		}
		ComponentConstructionHelper& SetChildSlot(
			const VectorUiCoord2& in_child_size,
			const VectorUiCoord2& in_child_pivot,
			const VectorUiCoord2& in_attach_point
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
			const std::function<void(const UiComponentResourceNodeGroup&)>& in_click_callback_or_none = {},
			const bool in_has_input_rollover_accumulate = false
		)
		{
			_has_input = true;
			_input_click_callback = in_click_callback_or_none;
			_has_input_rollover_accumulate = in_has_input_rollover_accumulate;
			return *this;
		}

		ComponentConstructionHelper& SetForInputStateFlag(
			const TUiInputStateFlag in_for_input_state_flag
		)
		{
			_has_for_input_flag = true;
			_for_input_state_flag = in_for_input_state_flag;
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

		ComponentConstructionHelper& SetGradientFill(
			const TGradientFillConstantBuffer& in_gradient_fill_constant_buffer
		)
		{
			_has_gradient_fill = true;
			_gradient_fill_constant_buffer = in_gradient_fill_constant_buffer;
			return *this;
		}
	};

	UiComponentResourceNodeGroup MakeComponentResourceGroup(
		DscDag::DagCollection& in_dag_collection,
		const ComponentConstructionHelper& in_construction_helper,
		DscDag::NodeToken in_ui_scale,
		DscDag::NodeToken in_avaliable_size
	);
	ComponentConstructionHelper MakeComponentDebugGrid();
	ComponentConstructionHelper MakeComponentFill(const DscCommon::VectorFloat4& in_colour);
	ComponentConstructionHelper MakeComponentGradientFill(
		const TGradientFillConstantBuffer& in_gradient_fill
		);
	ComponentConstructionHelper MakeComponentButton(
		const std::function<void(const UiComponentResourceNodeGroup&)>& in_click_callback_or_none = {},
		const bool in_has_input_rollover_accumulate = false
		);
	ComponentConstructionHelper MakeComponentImage(const std::shared_ptr<DscRenderResource::ShaderResource>& in_texture);
	ComponentConstructionHelper MakeComponentCanvas();
	ComponentConstructionHelper MakeComponentText(
		const std::shared_ptr<DscText::TextRun>& in_text_run,
		DscText::TextManager* const in_text_manager, // so, either the text manager needs to be told to upload the glyph texture before draw and we can grab the text shader pointer, or our draw method needs a ref to the text manager
		const bool in_has_scroll = true
		);
	ComponentConstructionHelper MakeComponentStack(
		const TUiFlow in_flow_direction,
		const UiCoord& in_gap,
		const bool in_desired_size_from_children_max = true,
		const bool in_has_scroll = true
		);

}
