/**
 * =============================================================================
 * Source Python
 * Copyright (C) 2012-2015 Source Python Development Team.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, the Source Python Team gives you permission
 * to link the code of this program (as well as its derivative works) to
 * "Half-Life 2," the "Source Engine," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, the Source.Python
 * Development Team grants this exception to all derivative works.
 *
 * This file has been modified from its original form, under the terms of GNU
 * General Public License, version 3.0.
 */

#include "core/function.h"

#include "core/log.h"
#include "dyncall/dyncall/dyncall.h"
#include "funchook.h"

namespace counterstrikesharp {

DCCallVM* g_pCallVM = dcNewCallVM(4096);

// ============================================================================
// >> GetDynCallConvention
// ============================================================================
int GetDynCallConvention(Convention_t eConv)
{
    switch (eConv) {
    case CONV_CUSTOM:
        return -1;
    case CONV_CDECL:
        return DC_CALL_C_DEFAULT;
    case CONV_THISCALL:
#ifdef _WIN32
        return DC_CALL_C_X86_WIN32_THIS_MS;
#else
        return DC_CALL_C_X86_WIN32_THIS_GNU;
#endif
#ifdef _WIN32
    case CONV_STDCALL:
        return DC_CALL_C_X86_WIN32_STD;
    case CONV_FASTCALL:
        return DC_CALL_C_X86_WIN32_FAST_MS;
#endif
    }

    return -1;
}

ValveFunction::ValveFunction(void* ulAddr, Convention_t callingConvention,
                             std::vector<DataType_t> args, DataType_t returnType)
    : m_ulAddr(ulAddr)
{
    m_Args = args;

    m_eReturnType = returnType;

    m_eCallingConvention = callingConvention;

    m_iCallingConvention = GetDynCallConvention(m_eCallingConvention);
}

ValveFunction::ValveFunction(void* ulAddr, Convention_t callingConvention, DataType_t* args,
                             int argCount, DataType_t returnType)
    : m_ulAddr(ulAddr)

{
    m_Args = std::vector<DataType_t>(args, args + argCount);
    m_eReturnType = returnType;

    m_eCallingConvention = callingConvention;
    m_iCallingConvention = GetDynCallConvention(m_eCallingConvention);
}

ValveFunction::~ValveFunction() {}

bool ValveFunction::IsCallable()
{
    return (m_eCallingConvention != CONV_CUSTOM) && (m_iCallingConvention != -1);
}

// bool ValveFunction::IsHookable() { return m_pCallingConvention != NULL; }
//
// bool ValveFunction::IsHooked() { return GetHookManager()->FindHook((void*)m_ulAddr) != NULL; }
//
// CHook* ValveFunction::GetHook() { return GetHookManager()->FindHook((void*)m_ulAddr); }

// ValveFunction* ValveFunction::GetTrampoline() {
//     CHook* pHook = GetHookManager()->FindHook((void*)m_ulAddr);
//     if (!pHook) return nullptr;
//
//     return new ValveFunction((unsigned long)pHook->m_pTrampoline, m_eCallingConvention, m_Args,
//                              m_eReturnType);
// }

template <class ReturnType, class Function>
ReturnType CallHelper(Function func, DCCallVM* vm, void* addr)
{
    ReturnType result;
    result = (ReturnType)func(vm, (void*)addr);
    return result;
}

void CallHelperVoid(DCCallVM* vm, void* addr) { dcCallVoid(vm, (void*)addr); }

void ValveFunction::Call(ScriptContext& script_context, int offset)
{
    if (!IsCallable())
        return;

    dcReset(g_pCallVM);
    dcMode(g_pCallVM, m_iCallingConvention);

    for (size_t i = 0; i < m_Args.size(); i++) {
        int contextIndex = i + offset;
        switch (m_Args[i]) {
        case DATA_TYPE_BOOL:
            dcArgBool(g_pCallVM, script_context.GetArgument<bool>(contextIndex));
            break;
        case DATA_TYPE_CHAR:
            dcArgChar(g_pCallVM, script_context.GetArgument<char>(contextIndex));
            break;
        case DATA_TYPE_UCHAR:
            dcArgChar(g_pCallVM, script_context.GetArgument<unsigned char>(contextIndex));
            break;
        case DATA_TYPE_SHORT:
            dcArgShort(g_pCallVM, script_context.GetArgument<short>(contextIndex));
            break;
        case DATA_TYPE_USHORT:
            dcArgShort(g_pCallVM, script_context.GetArgument<unsigned short>(contextIndex));
            break;
        case DATA_TYPE_INT:
            dcArgInt(g_pCallVM, script_context.GetArgument<int>(contextIndex));
            break;
        case DATA_TYPE_UINT:
            dcArgInt(g_pCallVM, script_context.GetArgument<unsigned int>(contextIndex));
            break;
        case DATA_TYPE_LONG:
            dcArgLong(g_pCallVM, script_context.GetArgument<long>(contextIndex));
            break;
        case DATA_TYPE_ULONG:
            dcArgLong(g_pCallVM, script_context.GetArgument<unsigned long>(contextIndex));
            break;
        case DATA_TYPE_LONG_LONG:
            dcArgLongLong(g_pCallVM, script_context.GetArgument<long long>(contextIndex));
            break;
        case DATA_TYPE_ULONG_LONG:
            dcArgLongLong(g_pCallVM, script_context.GetArgument<unsigned long long>(contextIndex));
            break;
        case DATA_TYPE_FLOAT:
            dcArgFloat(g_pCallVM, script_context.GetArgument<float>(contextIndex));
            break;
        case DATA_TYPE_DOUBLE:
            dcArgDouble(g_pCallVM, script_context.GetArgument<double>(contextIndex));
            break;
        case DATA_TYPE_POINTER:
            dcArgPointer(g_pCallVM, script_context.GetArgument<void*>(contextIndex));
            break;
        case DATA_TYPE_STRING:
            dcArgPointer(g_pCallVM, (void*)script_context.GetArgument<const char*>(contextIndex));
            break;
        default:
            assert(!"Unknown function parameter type!");
            break;
        }
    }

    switch (m_eReturnType) {
    case DATA_TYPE_VOID:
        CallHelperVoid(g_pCallVM, m_ulAddr);
        break;
    case DATA_TYPE_BOOL:
        script_context.SetResult(CallHelper<bool>(dcCallBool, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_CHAR:
        script_context.SetResult(CallHelper<char>(dcCallChar, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_UCHAR:
        script_context.SetResult(CallHelper<unsigned char>(dcCallChar, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_SHORT:
        script_context.SetResult(CallHelper<short>(dcCallShort, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_USHORT:
        script_context.SetResult(CallHelper<unsigned short>(dcCallShort, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_INT:
        script_context.SetResult(CallHelper<int>(dcCallInt, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_UINT:
        script_context.SetResult(CallHelper<unsigned int>(dcCallInt, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_LONG:
        script_context.SetResult(CallHelper<long>(dcCallLong, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_ULONG:
        script_context.SetResult(CallHelper<unsigned long>(dcCallLong, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_LONG_LONG:
        script_context.SetResult(CallHelper<long long>(dcCallLongLong, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_ULONG_LONG:
        script_context.SetResult(
            CallHelper<unsigned long long>(dcCallLongLong, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_FLOAT:
        script_context.SetResult(CallHelper<float>(dcCallFloat, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_DOUBLE:
        script_context.SetResult(CallHelper<double>(dcCallDouble, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_POINTER:
        script_context.SetResult(CallHelper<void*>(dcCallPointer, g_pCallVM, m_ulAddr));
        break;
    case DATA_TYPE_STRING:
        script_context.SetResult(CallHelper<const char*>(dcCallPointer, g_pCallVM, m_ulAddr));
        break;
    default:
        assert(!"Unknown function return type!");
        break;
    }
}

void prehook(funchook_info_t* info)
{
    CSSHARP_CORE_INFO("All good {}", (void*)info->user_data);
    auto vf = ((ValveFunction*)info->user_data);
    auto pEntityInstance = *(CEntityInstance**)funchook_arg_get_int_reg_addr(info->arg_handle, 0);

    CSSHARP_CORE_INFO(
        "Valve Function address: {}, original target func: {}, target func {}, hook {}, tramp {}",
        ((ValveFunction*)info->user_data)->m_ulAddr, info->original_target_func, info->target_func,
        info->hook_func, info->trampoline_func);
    auto func = (Remove)((ValveFunction*)info->user_data)->m_ulAddr;

    vf->m_precallback->Reset();

    std::vector<void*> argValues;

    for (size_t i = 0; i < vf->m_Args.size(); i++) {
        CSSHARP_CORE_INFO("Pushing callback arg {} type {}", i, vf->m_Args[i]);
        switch (vf->m_Args[i]) {
        case DATA_TYPE_BOOL:
            argValues.push_back((void*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            vf->m_precallback->ScriptContext().Push(
                *(bool*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_CHAR:
            vf->m_precallback->ScriptContext().Push(
                *(char*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_UCHAR:
            vf->m_precallback->ScriptContext().Push(
                *(unsigned char*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_SHORT:
            vf->m_precallback->ScriptContext().Push(
                *(short*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_USHORT:
            vf->m_precallback->ScriptContext().Push(
                *(unsigned short*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_INT:
            vf->m_precallback->ScriptContext().Push(
                *(int*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_UINT:
            vf->m_precallback->ScriptContext().Push(
                *(unsigned int*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_LONG:
            vf->m_precallback->ScriptContext().Push(
                *(long*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_ULONG:
            vf->m_precallback->ScriptContext().Push(
                *(unsigned long*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_LONG_LONG:
            vf->m_precallback->ScriptContext().Push(
                *(long long*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_ULONG_LONG:
            vf->m_precallback->ScriptContext().Push(
                *(unsigned long long*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_FLOAT:
            vf->m_precallback->ScriptContext().Push(
                *(float*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_DOUBLE:
            vf->m_precallback->ScriptContext().Push(
                *(double*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_POINTER:
            vf->m_precallback->ScriptContext().Push(
                *(void**)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        case DATA_TYPE_STRING:
            vf->m_precallback->ScriptContext().Push(
                *(const char**)funchook_arg_get_int_reg_addr(info->arg_handle, i));
            break;
        default:
            assert(!"Unknown function parameter type!");
            break;
        }
    }

    bool shouldFireOriginal = true;
    for (auto fnMethodToCall : vf->m_precallback->GetFunctions()) {
        if (!fnMethodToCall)
            continue;
        fnMethodToCall(&vf->m_precallback->ScriptContextStruct());

        auto result = vf->m_precallback->ScriptContext().GetResult<HookResult>();
        CSSHARP_CORE_INFO("Received hook callback result of {}", result);

        if (result >= HookResult::Stop) {
            return;
        }

        if (result >= HookResult::Handled) {
            shouldFireOriginal = false;
        }
    }
    /*
    if (shouldFireOriginal) {
        dcReset(g_pCallVM);
        dcMode(g_pCallVM, vf->m_iCallingConvention);

        for (size_t i = 0; i < vf->m_Args.size(); i++) {
            CSSHARP_CORE_INFO("Pushing orig call arg {} type {}", i, vf->m_Args[i]);

            switch (vf->m_Args[i]) {
            case DATA_TYPE_BOOL:
                dcArgBool(g_pCallVM, *(bool*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_CHAR:
                dcArgChar(g_pCallVM, *(char*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_UCHAR:
                dcArgChar(g_pCallVM,
                          *(unsigned char*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_SHORT:
                dcArgShort(g_pCallVM, *(short*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_USHORT:
                dcArgShort(g_pCallVM,
                           *(unsigned short*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_INT:
                dcArgInt(g_pCallVM, *(int*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_UINT:
                dcArgInt(g_pCallVM,
                         *(unsigned int*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_LONG:
                dcArgLong(g_pCallVM, *(long*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_ULONG:
                dcArgLong(g_pCallVM,
                          *(unsigned long*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_LONG_LONG:
                dcArgLongLong(g_pCallVM,
                              *(long long*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_ULONG_LONG:
                dcArgLongLong(g_pCallVM, *(unsigned long long*)funchook_arg_get_int_reg_addr(
                                             info->arg_handle, i));
                break;
            case DATA_TYPE_FLOAT:
                dcArgFloat(g_pCallVM, *(float*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_DOUBLE:
                dcArgDouble(g_pCallVM,
                            *(double*)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_POINTER:
                dcArgPointer(g_pCallVM,
                             *(void**)funchook_arg_get_int_reg_addr(info->arg_handle, i));
                break;
            case DATA_TYPE_STRING:
                dcArgPointer(g_pCallVM, (void*)*(const char**)funchook_arg_get_int_reg_addr(
                                            info->arg_handle, i));
                break;
            default:
                assert(!"Unknown function parameter type!");
                break;
            }
        }

        CSSHARP_CORE_INFO("Executing orig {}", vf->m_eReturnType);
        switch (vf->m_eReturnType) {
        case DATA_TYPE_VOID:
            CallHelperVoid(g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_BOOL:
            CallHelper<bool>(dcCallBool, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_CHAR:
            CallHelper<char>(dcCallChar, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_UCHAR:
            CallHelper<unsigned char>(dcCallChar, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_SHORT:
            CallHelper<short>(dcCallShort, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_USHORT:
            CallHelper<unsigned short>(dcCallShort, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_INT:
            CallHelper<int>(dcCallInt, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_UINT:
            CallHelper<unsigned int>(dcCallInt, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_LONG:
            CallHelper<long>(dcCallLong, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_ULONG:
            CallHelper<unsigned long>(dcCallLong, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_LONG_LONG:
            CallHelper<long long>(dcCallLongLong, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_ULONG_LONG:

            CallHelper<unsigned long long>(dcCallLongLong, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_FLOAT:
            CallHelper<float>(dcCallFloat, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_DOUBLE:
            CallHelper<double>(dcCallDouble, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_POINTER:
            CallHelper<void*>(dcCallPointer, g_pCallVM, vf->m_ulAddr);
            break;
        case DATA_TYPE_STRING:
            CallHelper<const char*>(dcCallPointer, g_pCallVM, vf->m_ulAddr);
            break;
        default:
            assert(!"Unknown function return type!");
            break;
        }
    }
*/
}

static void thiscall_hook() {

}

void ValveFunction::AddHook(CallbackT callable)
{
    CSSHARP_CORE_INFO("Adding hook to {}", m_ulAddr);
    auto m_hook = funchook_create();
    funchook_params_t params = {0};
    params.prehook = prehook;
    params.flags = FUNCHOOK_FLAG_FASTCALL;
    params.user_data = this;

    auto rv = funchook_prepare_with_params(m_hook, (void**)&m_ulAddr, &params);
    CSSHARP_CORE_INFO("Prepared hook, {}", rv);

    // funchook_prepare(m_hook, (void**)&m_pHostSay, (void*)&DetourHostSay);
    rv = funchook_install(m_hook, 0);
    CSSHARP_CORE_INFO("Installed hook, {}", rv);

    m_precallback = globals::callbackManager.CreateCallback("");
    m_precallback->AddListener(callable);
}

//
// CHook* HookFunctionHelper(void* addr, ICallingConvention* pConv) {
//    CHook* result;
//    result = GetHookManager()->HookFunction(addr, pConv);
//    return result;
//}
//
// void ValveFunction::DeleteHook() {
//    CHook* pHook = GetHookManager()->FindHook((void*)m_ulAddr);
//    if (!pHook) return;
//
//    // Set the calling convention to NULL, because DynamicHooks will delete it
//    // otherwise.
//    pHook->m_pCallingConvention = NULL;
//    GetHookManager()->UnhookFunction((void*)m_ulAddr);
//}
//
// CHook* ValveFunction::AddHook(HookType_t eType, void* callable) {
//    if (!IsHookable()) return nullptr;
//
//    CHook* pHook = GetHookManager()->FindHook((void*)m_ulAddr);
//
//    if (!pHook) {
//        pHook = HookFunctionHelper((void*)m_ulAddr, m_pCallingConvention);
//
//        // DynamicHooks will handle our convention from there, regardless if we
//        // allocated it or not.
//        m_bAllocatedCallingConvention = false;
//    }
//
//    // Add the hook handler. If it's already added, it won't be added twice
//    pHook->AddCallback(eType, (HookHandlerFn*)(void*)callable);
//
//    return pHook;
//}

} // namespace counterstrikesharp