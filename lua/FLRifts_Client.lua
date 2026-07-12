-- FL Rifts countdown and wave UI, delivered to the WoW client through AIO.

local AIO = AIO or require("AIO")

if AIO.AddAddon() then
	return
end

local ADDON_PREFIX = "FLRIFTS"
local COUNTDOWN_SECONDS = 120
local frame = _G.FLRiftsStatusFrame
local bar
local statusText
local endTime

local function FormatTime(seconds)
	seconds = math.max(0, math.ceil(seconds))
	return string.format("%02d:%02d", math.floor(seconds / 60),
		seconds % 60)
end

local function CreateStatusFrame()
	if frame then
		bar = frame.Bar
		statusText = frame.StatusText
		return
	end

	frame = CreateFrame("Frame", "FLRiftsStatusFrame", UIParent)
	frame:SetSize(260, 30)
	frame:SetPoint("TOP", UIParent, "TOP", 0, -45)
	frame:SetFrameStrata("HIGH")
	frame:SetClampedToScreen(true)
	frame:SetMovable(true)
	frame:EnableMouse(true)
	frame:RegisterForDrag("RightButton")
	frame:SetScript("OnDragStart", function(self)
		self:StartMoving()
	end)
	frame:SetScript("OnDragStop", function(self)
		self:StopMovingOrSizing()
	end)
	frame:SetBackdrop({
		bgFile = "Interface/Tooltips/UI-Tooltip-Background",
		edgeFile = "Interface/Tooltips/UI-Tooltip-Border",
		tile = true,
		tileSize = 16,
		edgeSize = 10,
		insets = { left = 3, right = 3, top = 3, bottom = 3 },
	})
	frame:SetBackdropColor(0.04, 0.01, 0.08, 0.9)
	frame:SetBackdropBorderColor(0.55, 0.2, 0.8, 1)

	bar = CreateFrame("StatusBar", nil, frame)
	bar:SetPoint("TOPLEFT", frame, "TOPLEFT", 5, -5)
	bar:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -5, 5)
	bar:SetStatusBarTexture("Interface/TargetingFrame/UI-StatusBar")
	bar:SetStatusBarColor(0.45, 0.08, 0.7, 0.95)

	local background = bar:CreateTexture(nil, "BACKGROUND")
	background:SetAllPoints()
	background:SetTexture(0.08, 0.08, 0.08, 0.9)

	statusText = bar:CreateFontString(nil, "OVERLAY", "GameFontNormal")
	statusText:SetPoint("CENTER")
	statusText:SetTextColor(1, 0.82, 0, 1)

	frame.Bar = bar
	frame.StatusText = statusText
	frame:Hide()
	AIO.SavePosition(frame, true)
end

local function ShowCountdown(remainingSeconds)
	CreateStatusFrame()
	remainingSeconds = tonumber(remainingSeconds) or 0
	endTime = GetTime() + remainingSeconds
	bar:SetMinMaxValues(0, COUNTDOWN_SECONDS)
	bar:SetValue(remainingSeconds)
	bar:SetStatusBarColor(0.45, 0.08, 0.7, 0.95)
	statusText:SetText("Rift begins in " .. FormatTime(remainingSeconds))
	frame:Show()
end

local function ShowWave(wave, enemies, isBoss)
	CreateStatusFrame()
	endTime = nil
	wave = tonumber(wave) or 1
	enemies = tonumber(enemies) or 0
	bar:SetMinMaxValues(0, 3)
	bar:SetValue(wave)
	bar:SetStatusBarColor(0.65, 0.12, 0.12, 0.95)

	local waveName = isBoss == "1" and "Boss wave" or
		("Wave " .. wave .. "/3")
	local enemyLabel = enemies == 1 and " enemy" or " enemies"
	statusText:SetText(waveName .. " - " .. enemies .. enemyLabel)
	frame:Show()
end

local function HandleRiftMessage(message)
	local command, first, second, third = strsplit("|", message)
	if command == "COUNTDOWN" then
		ShowCountdown(first)
	elseif command == "WAVE" then
		ShowWave(first, second, third)
	elseif command == "HIDE" then
		endTime = nil
		if frame then
			frame:Hide()
		end
	end
end

CreateStatusFrame()
frame:SetScript("OnUpdate", function()
	if not endTime then
		return
	end

	local remaining = endTime - GetTime()
	if remaining <= 0 then
		endTime = nil
		bar:SetMinMaxValues(0, 1)
		bar:SetValue(1)
		bar:SetStatusBarColor(0.8, 0.35, 0.05, 0.95)
		statusText:SetText("Rift invasion starting...")
		return
	end

	bar:SetValue(remaining)
	statusText:SetText("Rift begins in " .. FormatTime(remaining))
end)

local eventFrame = _G.FLRiftsStatusEventFrame or
	CreateFrame("Frame", "FLRiftsStatusEventFrame")
eventFrame:RegisterEvent("CHAT_MSG_ADDON")
eventFrame:SetScript("OnEvent", function(self, event, prefix, message)
	if event == "CHAT_MSG_ADDON" and prefix == ADDON_PREFIX then
		HandleRiftMessage(message)
	end
end)

SendAddonMessage(ADDON_PREFIX, "REQUEST", "WHISPER", UnitName("player"))
