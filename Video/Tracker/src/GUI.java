import java.awt.EventQueue;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.DefaultComboBoxModel;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.SwingConstants;
import javax.swing.border.EmptyBorder;

import processing.core.PApplet;

@SuppressWarnings("serial")
public class GUI extends JFrame {
	public static GUI theGUI;
	private JPanel contentPane;
	private JCheckBox drawMasks, drawBounds;
	@SuppressWarnings("rawtypes")
	private JComboBox appSelect;
	private boolean initialized=false;
	private JCheckBox useMasks;
	private JCheckBox showProjectors;
	private JTextArea fps;
	private JLabel lblFps;
	
	public static void start() {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					theGUI = new GUI();
					theGUI.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the frame.
	 */
	@SuppressWarnings({ "rawtypes", "unchecked" })
	public GUI() {
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 450, 300);
		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		GridBagLayout gbl_contentPane = new GridBagLayout();
		gbl_contentPane.columnWidths = new int[]{66, 114, 64, 32, 15, 72, 0};
		gbl_contentPane.rowHeights = new int[]{23, 27, 0, 0, 0, 0, 0, 0, 0, 0};
		gbl_contentPane.columnWeights = new double[]{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Double.MIN_VALUE};
		gbl_contentPane.rowWeights = new double[]{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Double.MIN_VALUE};
		contentPane.setLayout(gbl_contentPane);
		
		drawBounds = new JCheckBox("Draw Bounds");
		drawBounds.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				Tracker.theTracker.drawBounds=drawBounds.isSelected();
				PApplet.println("drawBounds="+drawBounds.toString());
			}
		});
		GridBagConstraints gbc_drawBounds = new GridBagConstraints();
		gbc_drawBounds.anchor = GridBagConstraints.NORTHWEST;
		gbc_drawBounds.insets = new Insets(0, 0, 5, 5);
		gbc_drawBounds.gridx = 1;
		gbc_drawBounds.gridy = 0;
		contentPane.add(drawBounds, gbc_drawBounds);
		
		showProjectors = new JCheckBox("Show Projectors");
		showProjectors.setHorizontalAlignment(SwingConstants.LEFT);
		showProjectors.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				Tracker.theTracker.showProjectors=showProjectors.isSelected();
			}
		});
		GridBagConstraints gbc_showProjectors = new GridBagConstraints();
		gbc_showProjectors.anchor = GridBagConstraints.EAST;
		gbc_showProjectors.insets = new Insets(0, 0, 5, 5);
		gbc_showProjectors.gridx = 1;
		gbc_showProjectors.gridy = 1;
		contentPane.add(showProjectors, gbc_showProjectors);

		
		String visnames[]=new String[Tracker.vis.length];
		for (int i=0;i<Tracker.vis.length;i++)
			visnames[i]=Tracker.vis[i].name;
		
		appSelect = new JComboBox();
		appSelect.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				Tracker.theTracker.setapp(appSelect.getSelectedIndex());
			}
		});
		
		useMasks = new JCheckBox("Use Mask");
		useMasks.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				Tracker.theTracker.useMasks=useMasks.isSelected();
			}
		});
		GridBagConstraints gbc_useMasks = new GridBagConstraints();
		gbc_useMasks.anchor = GridBagConstraints.NORTHWEST;
		gbc_useMasks.insets = new Insets(0, 0, 5, 5);
		gbc_useMasks.gridwidth = 2;
		gbc_useMasks.gridx = 1;
		gbc_useMasks.gridy = 2;
		contentPane.add(useMasks, gbc_useMasks);
		
		drawMasks = new JCheckBox("Draw Mask");
		drawMasks.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				Tracker.theTracker.drawMasks=drawMasks.isSelected();
			}
		});
		GridBagConstraints gbc_drawMasks = new GridBagConstraints();
		gbc_drawMasks.anchor = GridBagConstraints.NORTHWEST;
		gbc_drawMasks.insets = new Insets(0, 0, 5, 5);
		gbc_drawMasks.gridwidth = 2;
		gbc_drawMasks.gridx = 1;
		gbc_drawMasks.gridy = 3;
		contentPane.add(drawMasks, gbc_drawMasks);
		appSelect.setModel(new DefaultComboBoxModel(visnames));
		GridBagConstraints gbc_appSelect = new GridBagConstraints();
		gbc_appSelect.anchor = GridBagConstraints.NORTHWEST;
		gbc_appSelect.insets = new Insets(0, 0, 5, 5);
		gbc_appSelect.gridwidth = 2;
		gbc_appSelect.gridx = 1;
		gbc_appSelect.gridy = 4;
		contentPane.add(appSelect, gbc_appSelect);
		
		lblFps = new JLabel("FPS:");
		GridBagConstraints gbc_lblFps = new GridBagConstraints();
		gbc_lblFps.insets = new Insets(0, 0, 0, 5);
		gbc_lblFps.gridx = 4;
		gbc_lblFps.gridy = 8;
		contentPane.add(lblFps, gbc_lblFps);
		
		fps=new JTextArea("FPS");
		GridBagConstraints gbc_fps = new GridBagConstraints();
		gbc_fps.gridx = 5;
		gbc_fps.gridy = 8;
		contentPane.add(fps, gbc_fps);
		initialized=true;
		update();
	}
	
	void updateFPS() {
		fps.setText(String.format("%.0f", Tracker.theTracker.avgFrameRate));
	}
	
	void update() {
		PApplet.println("update called");
		if (!initialized) {
			PApplet.println("update when not initialized");
			return;
		}
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					PApplet.println("update run");
					drawMasks.setSelected(Tracker.theTracker.drawMasks);
					drawBounds.setSelected(Tracker.theTracker.drawBounds);
					useMasks.setSelected(Tracker.theTracker.useMasks);
					showProjectors.setSelected(Tracker.theTracker.showProjectors);
					appSelect.setSelectedIndex(Tracker.theTracker.currentvis);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}
}
