﻿using CounterStrikeSharp.API.Core;
using CounterStrikeSharp.API.Core.Attributes;
using CounterStrikeSharp.API.Core.Attributes.Registration;
using Microsoft.Extensions.Logging;

namespace WithEntityOutputHooks;

[MinimumApiVersion(80)]
public class WithEntityOutputHooksPlugin : BasePlugin
{
    public override string ModuleName => "Example: With Entity Output Hooks";
    public override string ModuleVersion => "1.0.0";
    public override string ModuleAuthor => "CounterStrikeSharp & Contributors";
    public override string ModuleDescription => "A simple plugin that showcases entity output hooks";

    public override void Load(bool hotReload)
    {
        HookEntityOutput("weapon_knife", "OnPlayerPickup", (string name, CEntityInstance activator, CEntityInstance caller, float delay) =>
        {
            Logger.LogInformation("weapon_knife called OnPlayerPickup ({name}, {activator}, {caller}, {delay})", name, activator.DesignerName, caller.DesignerName, delay);
            
            return HookResult.Continue;
        });
    }
    
    // Output hooks can use wildcards to match multiple entities
    [EntityOutputHook("*", "OnPlayerPickup")]
    public HookResult OnPickup(string name, CEntityInstance activator, CEntityInstance caller, float delay)
    {
        Logger.LogInformation("[EntityOutputHook Attribute] Called OnPlayerPickup ({name}, {activator}, {caller}, {delay})", name, activator.DesignerName, caller.DesignerName, delay);

        return HookResult.Continue;
    }
    
    // Output hooks can use wildcards to match multiple output names
    [EntityOutputHook("func_buyzone", "*")]
    public HookResult OnTouchStart(string name, CEntityInstance activator, CEntityInstance caller, float delay)
    {
        Logger.LogInformation("[EntityOutputHook Attribute] Buyzone called output ({name}, {activator}, {caller}, {delay})", name, activator.DesignerName, caller.DesignerName, delay);

        return HookResult.Continue;
    }
}