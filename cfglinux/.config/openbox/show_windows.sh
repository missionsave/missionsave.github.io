#!/bin/bash
trap 'wmctrl -c "MenuAnchor"' EXIT
wmctrl -r "MenuAnchor" -b add,below,skip_taskbar,skip_pager
wmctrl -r "MenuAnchor" -e 0,$menu_x,$menu_y,10,10

echo '<openbox_pipe_menu>'
wmctrl -l | while read -r line; do
    win_id=$(echo "$line" | awk '{print $1}')
    win_name=$(echo "$line" | cut -d ' ' -f 5-)
    echo "<item label=\"  $win_name\">"
    echo "  <action name=\"Execute\">"
    echo "    <command>wmctrl -ia $win_id</command>"
    echo "  </action>"
    echo "</item>"
done

cat <<EOF
    <separator/>
    <separator/>
        <item label="Hello World"> 
    </item>

    <item label="xterm">
      <action name="Execute">
        <command>xterm</command>
      </action>
    </item>
    
    <item label="Dash + tmux (com bash)">
  <action name="Execute">
    <execute>xterm -e dash -c 'exec tmux new-session -A -s dash bash'</execute>
  </action>
</item>

    <item label="tmux x86_64">
  <action name="Execute">
    <execute>xterm -e tmux new-session -A -s x86_64 bash</execute>
  </action>
</item>

 <item label="tmux aarch64">
  <action name="Execute">
    <execute>xterm -e tmux new-session -A -s aarch64 bash</execute>
  </action>
</item>

EOF


echo '</openbox_pipe_menu>'
