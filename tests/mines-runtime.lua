-- ETLab real-client integration test. Loaded only by -MineSmoke in a fresh devmap profile.
-- Geometry is real. For repeatability only, thrown grenades are placed beside the target;
-- their normal fuse, explosion, radius/cover checks and mine reactions run unchanged.
local now, resumeAt, job, done = 0, 0, nil, false
local plierPulses = {}
function et_WeaponFire(client, weapon)
    if client == 0 and weapon == et.WP_PLIERS then plierPulses[#plierPulses+1] = now end
end
local function get(id, field) return et.gentity_get(id, field) end
local function cmd(text) et.trap_SendConsoleCommand(et.EXEC_APPEND, text .. "\n") end
local function wait(ms) resumeAt = now + ms; coroutine.yield() end
local function check(value, message)
    assert(value, message)
    et.G_Print("ETLAB_MINES_CHECK: " .. message .. "\n")
end
local function targets()
    local mines, dynamite = {}, nil
    for id = 64, 1021 do
        if get(id, "inuse") == 1 and get(id, "targetname") == "etlab_practice_target" then
            if get(id, "s.weapon") == et.WP_LANDMINE then mines[#mines+1] = id
            elseif get(id, "s.weapon") == et.WP_DYNAMITE then dynamite = id end
        end
    end
    return mines, dynamite
end
local function aliveMine(id)
    return get(id, "inuse") == 1 and get(id, "s.eType") == 3 and get(id, "s.weapon") == et.WP_LANDMINE
end
local function aim(id, height, approach)
    local p, t = get(0, "ps.origin"), get(id, "r.currentOrigin")
    if approach then
        local dx, dy = p[1]-t[1], p[2]-t[2]
        local dist = math.sqrt(dx*dx+dy*dy)
        p = {t[1]+dx/dist*approach, t[2]+dy/dist*approach, t[3]+24}
    end
    local dx, dy = t[1]-p[1], t[2]-p[2]
    local dz = t[3]+height-p[3]-get(0, "ps.viewheight")
    local yaw = math.deg(math.atan(dy, dx))
    local pitch = -math.deg(math.atan(dz, math.sqrt(dx*dx+dy*dy)))
    cmd(string.format("setviewpos %.3f %.3f %.3f %.3f %.3f 0 0",p[1],p[2],p[3]-1,pitch,yaw))
    wait(800)
end
local function reset()
    cmd("-attack\n-zoom\netlab_targets")
    wait(700)
    local m,d = targets()
    check(#m == 3 and d ~= nil, "practice has three enemy mines and armed dynamite")
    local enemy = get(0,"sess.sessionTeam") == 1 and 2 or 1
    for _,id in ipairs(m) do check(get(id,"s.teamNum") == enemy, "practice mine is enemy-owned") end
    check(get(d,"s.effect1Time") > 0 and get(d,"s.teamNum") == enemy, "practice dynamite is armed and enemy-owned")
    return m,d
end
local function grenadeTest(team, weapon)
    if get(0,"sess.sessionTeam") ~= team then
        cmd(team == 1 and "team r 0 5 0" or "team b 0 0 0")
        wait(3500)
    end
    cmd("god\ngive ammo")
    local m = reset()
    local target = m[1]
    check(get(target,"s.modelindex2") == 0, "grenade target starts undiscovered")
    cmd("weaponbank 4")
    wait(1000)
    cmd("+attack")
    wait(250)
    cmd("-attack")
    local grenade
    for attempt=1,20 do
        wait(100)
        for id=64,1021 do
            if get(id,"inuse") == 1 and get(id,"s.eType") == 3 and get(id,"s.weapon") == weapon then grenade=id; break end
        end
        if grenade then break end
    end
    check(grenade ~= nil, "real hand grenade thrown: " .. weapon)
    local p = get(target,"r.currentOrigin"); p[3] = p[3]+8
    et.gentity_set(grenade,"s.pos",{trType=0,trTime=now,trDuration=0,trBase=p,trDelta={0,0,0}})
    et.gentity_set(grenade,"r.currentOrigin",p)
    et.trap_LinkEntity(grenade)
    check(get(target,"s.modelindex2") == 0, "mine still hidden immediately before grenade explosion")
    wait(4500)
    check(not aliveMine(target), "real grenade explosion destroys hidden mine: " .. weapon)
end
local function run()
    cmd("god")
    local m,d = reset()
    aim(m[1],0)
    cmd("+zoom")
    wait(4000)
    check(get(m[1],"s.modelindex2") == 1, "Soldier spots hidden mine with real binocular input")
    cmd("screenshotJPEG etlab-mines-spotted\n-zoom")
    wait(400)
    cmd("weaponbank 2")
    wait(900)
    check(get(0,"ps.weapon") == et.WP_AKIMBO_LUGER, "Akimbo selected after leaving binocular view")
    aim(m[1],20)
    cmd("+attack")
    wait(600)
    cmd("-attack")
    wait(800)
    check(not aliveMine(m[1]), "real pistol shots detonate discovered mine flag")
    -- Measure the production fire events, including each target's full defuse.
    -- Keep both teams/classes at identical skill levels and identical initial target health.
    local measurements = {}
    for _,skill in ipairs({0,4}) do
        measurements[skill] = {}
        for _,cls in ipairs({0,2}) do -- Soldier, Engineer
            cmd("-attack\nteam s")
            wait(700)
            cmd("team r " .. cls .. " 3 0")
            wait(2500)
            check(get(0,"sess.playerType") == cls and et.gentity_get(0,"ps.stats",et.STAT_PLAYER_CLASS) == cls, "actual class matches timing scenario")
            et.gentity_set(0,"sess.skill",et.SK_EXPLOSIVES_AND_CONSTRUCTION,skill)
            measurements[skill][cls] = {}
            for _,kind in ipairs({"dynamite","mine"}) do
                m,d = reset()
                local id = kind == "dynamite" and d or m[1]
                local xpBefore = et.gentity_get(0,"sess.skillpoints",et.SK_EXPLOSIVES_AND_CONSTRUCTION)
                aim(id,kind == "dynamite" and 4 or 0,45)
                cmd("weaponbank 5")
                wait(800)
                check(get(0,"ps.weapon") == et.WP_PLIERS, "pliers selected for timing test")
                plierPulses = {}
                local started=now
                cmd("+attack")
                while get(id,"inuse") == 1 and now-started < 12000 do wait(50) end
                cmd("-attack")
                check(get(id,"inuse") == 0, "completed " .. kind .. " defuse, class=" .. cls .. " skill=" .. skill)
                check(#plierPulses > 1, "measured actual pliers fire events")
                local span=plierPulses[#plierPulses]-plierPulses[1]
                local cadence=span/(#plierPulses-1)
                local expected=cls == 2 and 50 or 100
                check(math.abs(cadence-expected) < 5, "pliers cadence ms=" .. cadence .. " class=" .. cls .. " skill=" .. skill)
                measurements[skill][cls][kind]=span
                et.G_Print("ETLAB_DEFUSE_TIMING: class=" .. cls .. " skill=" .. skill .. " target=" .. kind .. " pulses=" .. #plierPulses .. " spanMs=" .. span .. "\n")
                if kind == "mine" then
                    check(et.gentity_get(0,"sess.skillpoints",et.SK_EXPLOSIVES_AND_CONSTRUCTION) >= xpBefore+4, "mine was defused rather than accidentally triggered")
                end
                wait(500)
            end
        end
        for _,kind in ipairs({"dynamite","mine"}) do
            local soldier=measurements[skill][0][kind]
            local engineer=measurements[skill][2][kind]
            check(math.abs(soldier-2*engineer) <= 150, "double defuse duration: " .. kind .. " skill=" .. skill)
        end
    end
    grenadeTest(1,et.WP_GRENADE_LAUNCHER)
    grenadeTest(2,et.WP_GRENADE_PINEAPPLE)
    et.G_Print("ETLAB_MINES_RUNTIME_PASS\n")
    done=true
    cmd("quit")
end
function et_InitGame() et.RegisterModname("ETLab mine runtime test") end
function et_RunFrame(time)
    now=time
    if done then return end
    if not job then
        if now < 12000 or get(0,"health") <= 0 then return end
        local m=targets()
        if #m ~= 3 then
            if now > 20000 then
                done=true
                et.G_Print("ETLAB_MINES_RUNTIME_FAIL: no complete practice field at spawn\n")
                cmd("quit")
            end
            return
        end
        job=coroutine.create(run)
    end
    if now < resumeAt then return end
    local ok,err=coroutine.resume(job)
    if not ok then
        done=true
        et.G_Print("ETLAB_MINES_RUNTIME_FAIL: " .. tostring(err) .. "\n")
        cmd("-attack\n-zoom\nscreenshotJPEG etlab-mines-failure\nwait 10\nquit")
    end
end
