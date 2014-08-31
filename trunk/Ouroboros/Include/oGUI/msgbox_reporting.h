// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Uses msgbox to report errors in a form that can be passed to reporting
#pragma once
#ifndef oGUI_msgbox_reporting_h
#define oGUI_msgbox_reporting_h

namespace ouro {

assert_action::value prompt_msgbox(const assert_context& _Assertion, const char* _Message);

} // namespace ouro

#endif
