#!/bin/bash
trap 'wmctrl -c "MenuAnchor"' EXIT
wmctrl -r "MenuAnchor" -b add,below,skip_taskbar,skip_pager
wmctrl -r "MenuAnchor" -e 0,$menu_x,$menu_y,10,10

echo '<openbox_pipe_menu>'

cat <<EOF
    <separator/>

<separator label="Favourites" />



 <item label="tmux $(uname -m)">
  <action name="Execute">
    <execute>xterm -e tmux new-session -A -s $(uname -m) bash</execute>
  </action>
</item>

    <item label="xterm" class="special-xterm-item">
      <action name="Execute">
        <command>xterm</command>
      </action>
    </item>
    
 

EOF





echo '<separator label="Open windows" />'
wmctrl -l | while read -r line; do
    win_id=$(echo "$line" | awk '{print $1}')
    win_name=$(echo "$line" | cut -d ' ' -f 5-)
    echo "<item label=\"  $win_name\">"
    echo "  <action name=\"Execute\">"
    echo "    <command>wmctrl -ia $win_id</command>"
    echo "  </action>"
    echo "</item>"
done



echo '</openbox_pipe_menu>'
