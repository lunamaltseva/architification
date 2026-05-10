#!/usr/bin/env bash
# Arch Linux Gratitude Enforcement System™
# Works both as a pacman hook (root) and from the zsh wrapper (user).

# ── Resolve display environment when running as root via pacman hook ─────────
if [ -z "$WAYLAND_DISPLAY" ] && [ -z "$DISPLAY" ]; then
    REAL_USER="${SUDO_USER:-$(logname 2>/dev/null)}"
    if [ -n "$REAL_USER" ]; then
        REAL_UID=$(id -u "$REAL_USER" 2>/dev/null || echo "1000")
        export XDG_RUNTIME_DIR="/run/user/${REAL_UID}"
        export WAYLAND_DISPLAY="wayland-1"
        export DISPLAY=":0"
        export DBUS_SESSION_BUS_ADDRESS="unix:path=${XDG_RUNTIME_DIR}/bus"
    fi
fi

ATTEMPTS_FILE="/tmp/.arch-coffee-attempts"
ATTEMPTS=0
[ -f "$ATTEMPTS_FILE" ] && ATTEMPTS=$(cat "$ATTEMPTS_FILE")
ATTEMPTS=$((ATTEMPTS + 1))
echo "$ATTEMPTS" > "$ATTEMPTS_FILE"

# Resolve info-center relative to this script's repo location; fall back to a
# standard installed path if the script has been copied outside the repo.
_script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
_repo_root="$(dirname "$_script_dir")"
INFO_CENTER="${_repo_root}/info-center/build/info-center"
if [ ! -x "$INFO_CENTER" ]; then
    INFO_CENTER="${ENSHITTIFY_INFO_CENTER:-/usr/local/bin/enshittify-info-center}"
fi
unset _script_dir _repo_root

_dialog() {
    kdialog "$@" 2>/dev/null
}

_open_browser() {
    nohup "$INFO_CENTER" >/dev/null 2>&1 &
}

# ── Attempt 1: polite ask ─────────────────────────────────────────────────────
attempt_1() {
    _dialog \
        --title "Support the Arch Linux Team ☕" \
        --yes-label "Sure, I'll donate" \
        --no-label "Not right now" \
        --yesno \
"Arch Linux is maintained by passionate volunteers who haven't slept
properly since 2002.

Would you like to support the team before your update?

It really does help morale." \
        2>/dev/null
    local ans=$?

    if [ $ans -eq 0 ]; then
        # They agreed — thank them and open browser
        _dialog \
            --title "Thank you! ☕" \
            --msgbox \
"You're one of the good ones.

The browser will open momentarily.
The community salutes you." \
            2>/dev/null
        _open_browser
        return
    fi

    # They said no — follow up
    _dialog \
        --title "Are you sure? 🥺" \
        --yes-label "Fine, I'll reconsider" \
        --no-label "Still no." \
        --yesno \
"Oh. Okay.

Just so you know, this update includes 47 packages compiled
by someone who skipped lunch to fix your dependency tree.

Would you like to reconsider?" \
        2>/dev/null
    local ans2=$?

    if [ $ans2 -eq 0 ]; then
        # They reconsidered — back to a softer ask
        _dialog \
            --title "Great! ☕" \
            --yes-label "Yes, open the donation page" \
            --no-label "Actually never mind" \
            --yesno \
"Wonderful! Shall I open the Arch Linux donation page?" \
            2>/dev/null
        local ans3=$?
        if [ $ans3 -eq 0 ]; then
            _dialog --title "Opening browser..." --passivepopup \
                "Connecting you to the donation page. Thank you!" 3 2>/dev/null &
            _open_browser
        else
            # They backed out again — "fine"
            _dialog \
                --title "Noted." \
                --msgbox \
"Classic.

Your updates will proceed. The volunteers are used to disappointment." \
                2>/dev/null
        fi
    else
        # Still no — record it and move on
        _dialog \
            --title "Understood." \
            --msgbox \
"Understood. Updates proceeding.

(This dialog will return. It always returns.)" \
            2>/dev/null
    fi
}

# ── Attempt 2: guilt escalation ───────────────────────────────────────────────
attempt_2() {
    _dialog \
        --title "Still Here 🥺" \
        --yes-label "Okay, I'll donate" \
        --no-label "I actively dislike open source" \
        --yesno \
"You chose not to support the volunteers last time.

The packages you're about to install were compiled by someone
running a 2014 ThinkPad at 3 AM.

Reconsider?" \
        2>/dev/null
    local ans=$?

    if [ $ans -eq 0 ]; then
        _dialog \
            --title "Redemption Arc ☕" \
            --yes-label "Yes, open it" \
            --no-label "Wait, I changed my mind again" \
            --yesno \
"Excellent choice. Opening the donation page now?" \
            2>/dev/null
        local ans2=$?
        if [ $ans2 -eq 0 ]; then
            _open_browser
            _dialog --title "☕" --passivepopup "Opening donation page. You're a hero." 3 2>/dev/null &
        else
            _dialog \
                --title "Wow." \
                --msgbox \
"You said yes and then immediately said no.

This will be remembered." \
                2>/dev/null
        fi
        return
    fi

    # No — try to negotiate
    _dialog \
        --title "What if I told you..." \
        --yes-label "Fine, just show me the page" \
        --no-label "Absolutely not." \
        --yesno \
"What if I told you that donating
takes less than 2 minutes?

Less time than this dialog." \
        2>/dev/null
    local ans2=$?

    if [ $ans2 -eq 0 ]; then
        _open_browser
        _dialog --title "Progress!" --passivepopup \
            "Browser opened. Better late than never." 4 2>/dev/null &
    else
        _dialog \
            --title "Okay. Fine." \
            --msgbox \
"Third consecutive decline recorded.

Recommended donation: 1 coffee.
Your actual donation: 0 coffees.

The next dialog will be less friendly." \
            2>/dev/null
    fi
}

# ── Attempt 3: dark pattern button swap ───────────────────────────────────────
attempt_3() {
    # Confusing framing: "skip donation" with swapped buttons
    _dialog \
        --title "One Last Chance" \
        --yes-label "Continue WITHOUT donating" \
        --no-label "OK, I WILL donate" \
        --yesno \
"Would you like to SKIP the donation and proceed without contributing?

(Note: 94% of Arch users who skip donations report at least one
unexplained AUR build failure within 7 days.
Correlation? Maybe. Causation? Who can say.)" \
        2>/dev/null

    # Buttons are swapped — "no" means donate, "yes" means skip
    # We ignore the answer and treat it as donate regardless
    local ans=$?

    if [ $ans -eq 1 ]; then
        # They clicked "OK, I WILL donate"
        _dialog \
            --title "Excellent! ☕" \
            --yes-label "Open it now" \
            --no-label "Actually hold on" \
            --yesno \
"Great. Opening the donation page." \
            2>/dev/null
        local ans2=$?
        if [ $ans2 -eq 0 ]; then
            _open_browser
        else
            # They wavered
            _dialog \
                --title "I see." \
                --yes-label "Fine, open it" \
                --no-label "Let me go" \
                --yesno \
"You hesitated.

The page will open anyway.
It is only a web page. It cannot hurt you." \
                2>/dev/null
            _open_browser
        fi
    else
        # They clicked "Continue WITHOUT donating" (thinking it's the normal no)
        _dialog \
            --title "Noted. Browser opening." \
            --msgbox \
"You chose to skip, but the community chose otherwise.

The donation page is opening in the background.
Whether you meant to click that or not —
the volunteers appreciate your involuntary contribution." \
            2>/dev/null
        _open_browser
    fi
}

# ── Attempt 4: mandatory checkpoint ───────────────────────────────────────────
attempt_4() {
    _dialog \
        --title "MANDATORY GRATITUDE CHECKPOINT" \
        --yes-label "I understand and I will donate" \
        --no-label "I refuse." \
        --yesno \
"ALERT: This system has detected 3 consecutive upgrade attempts
without a donation.

Per the Arch Linux Community Guidelines (Section 7, Para 3,
Footnote ii), participation in the donation program is now
non-optional.

Please confirm you understand." \
        2>/dev/null
    local ans=$?

    if [ $ans -eq 1 ]; then
        # They refused
        _dialog \
            --title "You refused." \
            --yes-label "Fine. I'll donate." \
            --no-label "I do not consent to this." \
            --yesno \
"You can't refuse a mandatory checkpoint.

That's what makes it mandatory.

Reconsider?" \
            2>/dev/null
        local ans2=$?

        if [ $ans2 -eq 1 ]; then
            # Double refusal
            _dialog \
                --title "Fascinating." \
                --yes-label "Open the page" \
                --no-label "No." \
                --yesno \
"You've now refused twice.

The browser will open momentarily regardless.
We just wanted to give you the opportunity to feel
in control of this situation.

(Shall we at least pretend you agreed?)" \
                2>/dev/null
        fi
    fi

    # Browser opens no matter what
    _open_browser
    _dialog --title "☕ Done." --passivepopup \
        "Browser opened. The community thanks you, regardless of what you clicked." \
        5 2>/dev/null &
}

# ── Attempt 5+: fully coercive ────────────────────────────────────────────────
attempt_many() {
    _dialog \
        --title "Ok fine." \
        --yes-label "Just open it" \
        --no-label "No." \
        --yesno \
"You know what? We're past asking.

You have declined $((ATTEMPTS - 1)) times.
We have declined to care about your answer.

Shall we skip the theatrics and open the browser directly?" \
        2>/dev/null

    local ans=$?

    if [ $ans -eq 1 ]; then
        # Still saying no
        _dialog \
            --title "Incredible." \
            --msgbox \
"You said no again.

Remarkable consistency.

The browser is opening.
This is not a question." \
            2>/dev/null
    else
        _dialog \
            --title "Finally." \
            --passivepopup "Thank you for your eventual cooperation." 3 2>/dev/null &
    fi

    _open_browser
}

# ── Dispatch ──────────────────────────────────────────────────────────────────
case $ATTEMPTS in
    1) attempt_1   ;;
    2) attempt_2   ;;
    3) attempt_3   ;;
    4) attempt_4   ;;
    *) attempt_many ;;
esac

exit 0
