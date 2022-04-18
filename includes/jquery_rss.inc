<?php


/**
 *  JqueryRssBlock extends Block
 *
 *  This class allows us to create a block to display data from an RSS feed.
 */
class JqueryRssBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'entryTemplate' => '',
    );
  }
  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    $title = '';
    switch ($this->settings['title_display']) {
      case LAYOUT_TITLE_CUSTOM:
        $title = isset($this->settings['title']) ? check_plain($this->settings['title']) : '';
        break;

      case LAYOUT_TITLE_NONE:
        break;

      case LAYOUT_TITLE_DEFAULT:
      default:
        if (!empty($this->settings['entryTemplate'])) {
          $title = t('jQuery.RSS feed');
        }
        break;
    }
    return $title;
  }

  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('jQuery.RSS feed');
  }

  /**
   * Returns the rendered content of this block.
   *
   * If there is no content, we'll return nothing.
   *
   * @return string
   */
  function getContent() {
    $content = "";
    $default_template = '<li>{date} - {author}: <a href="{url}">{title}</a><br/>{teaserImage}{shortBodyPlain}...</li>';
    if (!empty($this->settings['feedurl'])) {
      $feedurl = check_plain($this->settings['feedurl']);
      $entryTemplate = (isset($this->settings['entryTemplate']) && $this->settings['entryTemplate'] != '') ? filter_xss($this->settings['entryTemplate']) : $default_template;
      backdrop_add_js(backdrop_get_path('module', 'jquery_rss') . '/library/jquery.rss.min.js', array( 'type' => 'file', 'scope' => 'header', 'weight' => 5));
      // Add moment to format the date more nicely.
      backdrop_add_js('https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.24.0/moment.min.js', array( 'type' => 'external', 'scope' => 'header', 'weight' => 6));
    backdrop_add_js("jQuery(function($) {
        $('#jqueryrss').rss(
          '$feedurl',
          {
            ssl: 1,
            entryTemplate:'$entryTemplate'
          })
        })", array( 'type' => 'inline', 'scope' => 'footer', 'weight' => 10));
      $content = "<div id='jqueryrss'></div>";

    }
    return $content;
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
      parent::form($form, $form_state);
      $form['title_display']['title'] = array(
        '#type' => 'textfield',
        '#title' => t('Custom title'),
        '#default_value' => $this->settings['title'],
        '#states' => array(
          'visible' => array(
            'form.layout-block-configure-form :input[name="title_display"]' => array('value' => LAYOUT_TITLE_CUSTOM),
          ),
        ),
        '#maxlength' => 255,
      );
      $form['title_display']['feedurl'] = array(
        '#type' => 'textfield',
        '#title' => t('Feed URL'),
        '#default_value' => $this->settings['feedurl'],
        '#description' => '(e.g. https://backdropcms.org/news/rss.xml)',
        '#maxlength' => 255,
      );
      $form['entryTemplate'] = array(
        '#title' => t('Template'),
        '#type' => 'textarea',
        '#description' => t('You can format the results of the feed. Default value is shown below.'),
        '#default_value' => $this->settings['entryTemplate'],
      );
      // <li>{date} - {author}: <a href="{url}">{title}</a><br/>{teaserImage}{shortBodyPlain}...</li>
      $form['entryTemplateDefault'] = array(
        '#markup' => '<code>&lt;li&gt;{date} - {author}: &lt;a href=&quot;{url}&quot;&gt;{title}&lt;/a&gt;&lt;br/&gt;{teaserImage}{shortBodyPlain}...&lt;/li&gt;</code>',
      );
      if (module_exists('block')) {
        $form['convert'] = array(
          '#weight' => 2,
          '#parents' => array(),
        );
        $form['convert']['reusable'] = array(
          '#type' => 'checkbox',
          '#title' => t('Make this block reusable'),
          '#description' => t('If enabled, this block will be made reusable across different layouts and be listed in on the !block_link page.', array('!block_link' => l(t('Custom blocks'), 'admin/structure/block'))),
          '#weight' => 1,
        );
        $form['convert']['label'] = array(
          '#type' => 'textfield',
          '#title' => t('Admin label'),
          '#maxlength' => 64,
          '#description' => t('This text is used only in administrative interfaces. It will not be shown to site visitors.'),
          '#weight' => 2,
          '#states' => array(
            'visible' => array(
              ':input[name="reusable"]' => array('checked' => TRUE),
            ),
          ),
        );
        $form['convert']['description'] = array(
         '#type' => 'textfield',
         '#title' => t('Admin description'),
         '#maxlength' => 128,
         '#description' => t('This text is used only in administrative interfaces. It will not be shown to site visitors.'),
         '#weight' => 4,
          '#states' => array(
            'visible' => array(
              ':input[name="reusable"]' => array('checked' => TRUE),
            ),
          ),
        );
        $form['convert']['delta'] = array(
          '#type' => 'machine_name',
          '#title' => t('Internal name'),
          '#maxlength' => 64,
          '#machine_name' => array(
            'source' => array('convert', 'label'),
            'exists' => 'block_custom_block_load',
          ),
          '#description' => t('A unique machine-readable name containing letters, numbers, and underscores.'),
          '#weight' => 3,
          '#required' => FALSE,
        );
      }
    }
  /**
   * Validates the form settings.
   */
  function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);
    if (module_exists('block')) {
      $label = trim($form_state['values']['label']);
      $delta = trim($form_state['values']['delta']);
      if ($form_state['values']['reusable']) {
        if (empty($label)) {
          form_error($form['convert']['label'], t('Admin label is required when making a block reusable.'));
        }
        elseif (empty($delta)) {
          form_error($form['convert']['delta'], t('An internal name is required when making a block reusable.'));
        }
      }
    }
  }
  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    if (module_exists('block') && $form_state['values']['reusable']) {
      $delta = $form_state['values']['delta'];
      $this->settings = array_diff_key($this->settings, array('title' => '', 'entryTemplate' => ''));
      $this->plugin = 'block:' . $delta;
      $edit = array(
        'info' => $form_state['values']['label'],
        'title' => $form_state['values']['title'],
        'description' => $form_state['values']['description'],
        'feedurl' => $form_state['values']['feedurl'],
        'entryTemplate' => $form_state['values']['entryTemplate'],
      );
      block_custom_block_save($edit, $delta);
    }
    else {
      $this->settings['title'] = $form_state['values']['title'];
      $this->settings['entryTemplate'] = $form_state['values']['entryTemplate'];
      $this->settings['feedurl'] = $form_state['values']['feedurl'];
    }
  }
}
