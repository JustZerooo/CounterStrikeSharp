/*
 *  This file is part of CounterStrikeSharp.
 *  CounterStrikeSharp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CounterStrikeSharp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CounterStrikeSharp.  If not, see <https://www.gnu.org/licenses/>. *
 */

using System;
using System.IO;
using CounterStrikeSharp.API;
using CounterStrikeSharp.API.Core;
using CounterStrikeSharp.API.Core.Attributes;
using CounterStrikeSharp.API.Core.Attributes.Registration;
using CounterStrikeSharp.API.Modules.Commands;
using CounterStrikeSharp.API.Modules.Entities;
using CounterStrikeSharp.API.Modules.Events;
using CounterStrikeSharp.API.Modules.Memory;

namespace TestPlugin
{
    public class SamplePlugin : BasePlugin
    {
        public override string ModuleName => "Sample Plugin";
        public override string ModuleVersion => "v1.0.0";

        public override void Load(bool hotReload)
        {
            Console.WriteLine(
                $"Test Plugin has been loaded, and the hot reload flag was {hotReload}, path is {ModulePath}");

            // Register Game Event Handlers
            RegisterEventHandler<EventPlayerConnect>(GenericEventHandler);
            RegisterEventHandler<EventPlayerSpawn>(GenericEventHandler);
            RegisterEventHandler<EventPlayerBlind>(GenericEventHandler);
            RegisterEventHandler<EventBulletImpact>(@event =>
            {
                Log(@event.Userid.Handle.ToString());

                var steamId = NativeAPI.GetSchemaValueByName<ulong>(@event.Userid.Handle,
                    (int)DataType.DATA_TYPE_ULONG_LONG,
                    "CBasePlayerController", "m_steamID");

                var playerName = NativeAPI.GetSchemaValueByName<string>(@event.Userid.Handle,
                    (int)DataType.DATA_TYPE_STRING, "CBasePlayerController", "m_iszPlayerName");
                Log($"Found value {new SteamID(steamId)} for player {playerName}");
                Log($"{@event.Userid}, {@event.X},{@event.Y},{@event.Z}");
            });

            // Hook global listeners defined by CounterStrikeSharp
            RegisterListener<Listeners.OnMapStart>(mapName => { Log($"Map {mapName} has started!"); });
            RegisterListener<Listeners.OnClientConnect>((index, name, ip) =>
            {
                Log($"Client {name} from {ip} has connected!");
            });
            RegisterListener<Listeners.OnClientAuthorized>((index, id) =>
            {
                Log($"Client {index} with address {id}");
            });

            // You can use `ModuleDirectory` to get the directory of the plugin (for storing config files, saving database files etc.)
            File.WriteAllText(Path.Join(ModuleDirectory, "example.txt"),
                $"Test file created by TestPlugin at {DateTime.Now}");

            // ValveInterface provides pointers to loaded modules via Interface Name exposed from the engine (e.g. Source2Server001)
            var server = ValveInterface.Server;
            Log($"Server pointer found @ {server.Pointer:X}");

            // Execute a server command as if typed into the server console.
            Server.ExecuteCommand("find \"cssharp\"");

            // Adds a new server console command
            AddCommand("cssharp_info", "A test command",
                (clientIndex, info) =>
                {
                    Log($"CounterStrikeSharp - a test command was called by {clientIndex} with {info.ArgString}");
                });

            // Example vfunc call that usually gets the game event manager pointer
            // by calling the func at offset 91 then subtracting 8 from the result pointer.
            // This value is asserted against the native code that points to the same function.
            var virtualFunc = VirtualFunction.CreateFunc<IntPtr>(server.Pointer, 91);
            var result = virtualFunc.Invoke() - 8;
            Log($"Result of virtual func call is {result:X}");

            // 	inline void(FASTCALL *ClientPrint)(CBasePlayerController *player, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4);
            var sigVirtualFunc = VirtualFunction.Create<IntPtr, int, string, IntPtr, IntPtr, IntPtr, IntPtr>(
                server.Pointer,
                @"\x55\x48\x89\xE5\x41\x57\x49\x89\xCF\x41\x56\x49\x89\xD6\x41\x55\x41\x89\xF5\x41\x54\x4C\x8D\xA5\xA0\xFE\xFF\xFF");
        }

        [GameEventHandler]
        public void OnPlayerConnect(EventPlayerConnect @event)
        {
            Log($"Player {@event.Name} has connected!");
        }

        [ConsoleCommand("cssharp_attribute", "This is a custom attribute event")]
        public void OnCommand(int client, CommandInfo command)
        {
            Log("cssharp_attribute called!");
        }

        private void GenericEventHandler<T>(T @event) where T : GameEvent
        {
            Log($"Event found {@event.Handle:X}, event name: {@event.EventName}");
        }

        private void Log(string message)
        {
            Console.BackgroundColor = ConsoleColor.DarkGray;
            Console.ForegroundColor = ConsoleColor.DarkMagenta;
            Console.WriteLine(message);
            Console.ResetColor();
        }
    }
}