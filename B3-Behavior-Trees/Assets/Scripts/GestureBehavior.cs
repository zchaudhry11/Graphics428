using UnityEngine;
using TreeSharpPlus;

public class GestureBehavior : MonoBehaviour 
{
    //Transform of meeting points
    public Transform destination1; 
    public Transform destination2;
    public Transform destination3;
    public Transform destination4;

    //Characters that are performing behaviors
    public GameObject participant; 
    public GameObject friend1;
    public GameObject friend2;

    private float agentDistance = 0;

    private bool _arrived1 = false; //Flag raised when friend is near destination
    private bool _arrived2 = false;
    private bool _arrived3 = false;
    private bool _arrived4 = false;

    private int zone1 = 0; //Number of characters at meeting point
    private int zone2 = 0;
    private int zone3 = 0;
    private int zone4 = 0;

    private bool _conv1 = false; //Flag raised when conversation complete
    private bool _conv2 = false;
    private bool _conv3 = false;
    private bool _conv4 = false;

    private bool _haveBall = true; //Raised when character has ball, lowered when dropped

    //Mecanim of participants
    private BehaviorMecanim charMec; 
    private BehaviorMecanim friend1Mec;
    private BehaviorMecanim friend2Mec;

    private BehaviorAgent behaviorAgent;

    private GameObject[] characters;

    void Start()
    {
        charMec = participant.GetComponent<BehaviorMecanim>();
        friend1Mec = friend1.GetComponent<BehaviorMecanim>();

        behaviorAgent = new BehaviorAgent(this.BuildTreeRoot());
        BehaviorManager.Instance.Register(behaviorAgent);
        behaviorAgent.StartBehavior();
    }

    void Update()
    {
        zone1 = 0;
        zone2 = 0;
        zone3 = 0;
        zone4 = 0;

        //Spherecast to see if there are ever 2 or more characters at a meeting point
        RaycastHit[] hit;
        hit = Physics.SphereCastAll(destination1.position, 2f, transform.up, Mathf.Infinity);

        for (int i = 0; i < hit.Length; i++)
        {
            if (hit[i].transform.gameObject.tag == "Character")
            {
                zone1++;
            }
        }

        if (zone1 >= 2)
        {
            print("Agents met up at zone 1!");
            _arrived1 = true;

            //print(zone1);
        }

        RaycastHit[] hit2;
        hit2 = Physics.SphereCastAll(destination2.position, 2f, transform.up, Mathf.Infinity);

        for (int i = 0; i < hit2.Length; i++)
        {
            if (hit2[i].transform.gameObject.tag == "Character")
            {
                zone2++;
            }
        }

        if (zone2 >= 2)
        {
          //  print("Agents met up at zone 2!");
            _arrived2 = true;

            //print(zone2);
        }

        RaycastHit[] hit3;
        hit3 = Physics.SphereCastAll(destination3.position, 2f, transform.up, Mathf.Infinity);

        for (int i = 0; i < hit3.Length; i++)
        {
            if (hit3[i].transform.gameObject.tag == "Character")
            {
                zone3++;
            }
        }

        if (zone3 >= 2)
        {
           // print("Agents met up at zone 3!");
            _arrived3 = true;

            //print(zone3);
        }

        RaycastHit[] hit4;
        hit4 = Physics.SphereCastAll(destination4.position, 2f, transform.up, Mathf.Infinity);

        for (int i = 0; i < hit4.Length; i++)
        {
            if (hit4[i].transform.gameObject.tag == "Character")
            {
                zone4++;
            }
        }

        if (zone4 >= 2)
        {
           // print("Agents met up at zone 4!");
            _arrived4 = true;


            //print(zone4);
        }

        if (_haveBall == true)
        {

        }
        
    }

    protected Node ST_Approach(Transform target)
    {
        
        Val<Vector3> position = Val.V(() => target.position); //Convert position to type Val so it can be passed into tree

        Val<Vector3> friendPos = Val.V(() => friend1.transform.position);

        float rand = Random.Range(0, 10);
        rand = 8;
        if (rand > 3 && rand < 7)
        {
            return new Sequence(
                charMec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1))
                );
        }
        else if (rand < 3)
        {
            return new Sequence(
                friend1Mec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1))
                );
        }
        else if (rand >= 7)
        {
            return new Sequence(
                charMec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1)),
                friend1Mec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1)),
                //new Sequence(ST_Converse(1))
                new Sequence(ST_PickUp())
                );
        }

        return new Sequence(charMec.Node_GoTo(new Vector3(target.position.x, target.position.y, target.position.z - 1)), new LeafWait(250));
    }

    protected Node ST_Wave()
    {
        Val<string> waveAnim = Val.V(() => "Wave");
        Val<bool> playAnim = Val.V(() => true);

        return new Sequence(charMec.Node_HandAnimation(waveAnim, playAnim), new LeafWait(250));
    }

    protected Node ST_PickUp()
    {
        Val<string> cheerAnim = Val.V(() => "Cheer");
        Val<string> pickAnim = Val.V(() => "PickupLeft");

        Val<Vector3> partPos = Val.V(() => participant.transform.position);
        partPos.Fetch();
        GameObject ball = GetRightHandItem(participant);
        Vector3 ballPos = GameObject.FindGameObjectWithTag("BALL").transform.position;

        Val<Vector3> ballPosition = Val.V(() => ballPos);

        if (ball == null) //If you do not own a ball and walk past one with no parent, pick it up
        {
            return new Sequence(
                new Sequence(
                    charMec.Node_HandAnimation(pickAnim, true), new LeafWait(2110), charMec.Node_HandAnimation(pickAnim, false),
                    charMec.Node_HandAnimation(cheerAnim, true), new LeafWait(1180), charMec.Node_HandAnimation(cheerAnim, false)
                    )
                );
        }
        else //Already have a ball, so drop it
        {
            //ball.transform.parent = null;
           // ball.transform.position = new Vector3(partPos.Value.x, 0.1f, partPos.Value.z);

            return new Sequence(
                new Sequence(
                    charMec.Node_BodyAnimation(pickAnim, true), new LeafWait(2110), charMec.Node_BodyAnimation(pickAnim, false)
                    )
                );
        }



    }

    protected Node ST_Trade(int zone)
    {
        //Spherecast to get participants in animation
        Val<string> thinkAnim = Val.V(() => "Think");
        Val<string> wondAnim = Val.V(() => "Wonderful");
        Val<string> txtAnim = Val.V(() => "Texting");
        Val<string> cheerAnim = Val.V(() => "Cheer");
        Val<bool> _trade1 = Val.V(() => _arrived1);

        Val<Vector3> partPos = Val.V(() => participant.transform.position);
        Val<Vector3> f1Pos = Val.V(() => friend1.transform.position);

        Val<float> distance = Val.V(() => Vector3.Distance(partPos.Value, f1Pos.Value));

        GameObject ball = GetRightHandItem(participant);

        //Check number of people from spherecast
        if (zone == 1)
        {
            ball.SetActive(true);
            if (_trade1.Value == true)
            {
                ball.SetActive(true);
            }

           // rightHand.transform.GetChild(8).gameObject.SetActive(true);
            /*return new Sequence(
                    new Sequence(
                        charMec.Node_OrientTowards(f1Pos), friend1Mec.Node_OrientTowards(partPos)
                    ),
                    new Sequence(
                        charMec.Node_HandAnimation(thinkAnim, true), new LeafWait(1180), charMec.Node_HandAnimation(thinkAnim, false),
                        friend1Mec.Node_HandAnimation(txtAnim, true), new LeafWait(5000), friend1Mec.Node_HandAnimation(txtAnim, false),
                        charMec.Node_HandAnimation(wondAnim, true), new LeafWait(1190), charMec.Node_HandAnimation(wondAnim, false)
                        //friend1Mec.Node_HandAnimation(wondAnim, true), new LeafWait(1190), friend1Mec.Node_HandAnimation(wondAnim, false)
                        )
                );*/
        }
        else if (zone == 2)
        {

        }
        else if (zone == 3)
        {

        }
        else if (zone == 4)
        {

        }

        return null;
    }

    protected Node ST_Converse(int zone)
    {
        Val<string> thinkAnim = Val.V(() => "Think");
        Val<string> surpAnim = Val.V(() => "Surprised");
        Val<string> bcAnim = Val.V(() => "BeingCocky");
        Val<string> cheerAnim = Val.V(() => "Cheer");

        long dur = 1250;
        Val<long> duration = Val.V(() => dur);
        Val<long> duration2 = Val.V(() => dur*2);
        Val<Vector3> partPos = Val.V(() => participant.transform.position);
        Val<Vector3> f1Pos = Val.V(() => friend1.transform.position);
        
        //Check meeting point of characters
        if (zone == 1)
        {
            if ( _conv1 == false) //If there are exactly 2 characters
            {
                print("Executed first!");
                GameObject[] participants = GameObject.FindGameObjectsWithTag("Character");
                //ForEach<GameObject> characters = new ForEach<GameObject>(, participants);

                for (int i = 0; i < participants.Length-1; i++)
                {
                  //  characters.AddParticipant(participants[i]);
                }
                
                //_conv1 = true;
                /*return new Sequence(
                    new Sequence(
                        charMec.Node_OrientTowards(friend1.transform.position), friend1Mec.Node_OrientTowards(participant.transform.position)
                    ),
                    new Sequence(
                                 charMec.ST_PlayHandGesture(bcAnim, duration),
                                 friend1Mec.ST_PlayHandGesture(bcAnim, duration2),
                                 charMec.ST_PlayHandGesture(surpAnim, duration),
                                 friend1Mec.ST_PlayHandGesture(cheerAnim, duration)
                        ));*/
                return new Sequence(
                    new Sequence(
                        charMec.Node_OrientTowards(f1Pos), friend1Mec.Node_OrientTowards(partPos)
                    ),
                    new Sequence(
                        charMec.Node_HandAnimation(bcAnim, true), new LeafWait(1180), charMec.Node_HandAnimation(bcAnim, false),
                        friend1Mec.Node_HandAnimation(bcAnim, true), new LeafWait(1180), friend1Mec.Node_HandAnimation(bcAnim, false),
                        charMec.Node_HandAnimation(surpAnim, true), new LeafWait(4000), charMec.Node_HandAnimation(surpAnim, false),
                        friend1Mec.Node_HandAnimation(cheerAnim, true), new LeafWait(1180), friend1Mec.Node_HandAnimation(cheerAnim, false)
                        ));
            }
            else if (zone1 > 2) //If there are 3 or more
            {

            }
            else //Conversation finished, return failure
            {
                _conv1 = false;
                //return null;
            }
        }
        else if (zone == 2)
        {
            if (zone2 == 2)
            {
                return new Sequence(
                    new Sequence(
                        charMec.Node_OrientTowards(f1Pos), friend1Mec.Node_OrientTowards(partPos)
                    ),
                    new Sequence(
                        charMec.Node_HandAnimation(bcAnim, true), new LeafWait(1180), charMec.Node_HandAnimation(bcAnim, false),
                        friend1Mec.Node_HandAnimation(bcAnim, true), new LeafWait(1180), friend1Mec.Node_HandAnimation(bcAnim, false),
                        charMec.Node_HandAnimation(surpAnim, true), new LeafWait(4000), charMec.Node_HandAnimation(surpAnim, false),
                        friend1Mec.Node_HandAnimation(cheerAnim, true), new LeafWait(1180), friend1Mec.Node_HandAnimation(cheerAnim, false)
                        ));
            }
            else if (zone2 > 2)
            {

            }
        }
        else if (zone == 3)
        {
            if (zone3 == 2)
            {
                return new Sequence(
                    new Sequence(
                        charMec.Node_OrientTowards(f1Pos), friend1Mec.Node_OrientTowards(partPos)
                    ),
                    new Sequence(
                        charMec.Node_HandAnimation(bcAnim, true), new LeafWait(1180), charMec.Node_HandAnimation(bcAnim, false),
                        friend1Mec.Node_HandAnimation(bcAnim, true), new LeafWait(1180), friend1Mec.Node_HandAnimation(bcAnim, false),
                        charMec.Node_HandAnimation(surpAnim, true), new LeafWait(4000), charMec.Node_HandAnimation(surpAnim, false),
                        friend1Mec.Node_HandAnimation(cheerAnim, true), new LeafWait(1180), friend1Mec.Node_HandAnimation(cheerAnim, false)
                        ));
            }
            else if (zone3 > 2)
            {

            }
        }
        else if (zone == 4)
        {
            if (zone4 == 2)
            {
                return new Sequence(
                    new Sequence(
                        charMec.Node_OrientTowards(f1Pos), friend1Mec.Node_OrientTowards(partPos)
                    ),
                    new Sequence(
                        charMec.Node_HandAnimation(bcAnim, true), new LeafWait(1180), charMec.Node_HandAnimation(bcAnim, false),
                        friend1Mec.Node_HandAnimation(bcAnim, true), new LeafWait(1180), friend1Mec.Node_HandAnimation(bcAnim, false),
                        charMec.Node_HandAnimation(surpAnim, true), new LeafWait(4000), charMec.Node_HandAnimation(surpAnim, false),
                        friend1Mec.Node_HandAnimation(cheerAnim, true), new LeafWait(1180), friend1Mec.Node_HandAnimation(cheerAnim, false)
                        ));
            }
            else if (zone3 > 2)
            {

            }
        }

        return new SequenceParallel(charMec.Node_HandAnimation(thinkAnim, true), new LeafWait(250), charMec.Node_HandAnimation(surpAnim, true),
               new LeafWait(250), friend1Mec.Node_HandAnimation(bcAnim, true), new LeafWait(250), friend1Mec.Node_HandAnimation(cheerAnim, true)
                );
    }

    private GameObject GetRightHandItem(GameObject actor)
    {
        GameObject rightHand = participant.transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(1).transform.GetChild(1).transform.GetChild(0).transform.GetChild(4).transform.GetChild(0).transform.GetChild(1).transform.GetChild(2).gameObject;
        GameObject item = null;
        if (rightHand.transform.childCount >= 9)
        {
            item = rightHand.transform.GetChild(8).gameObject;
        }

        return item;
    }

    private GameObject GetRightHand(GameObject actor)
    {
        GameObject rightHand = participant.transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(0).transform.GetChild(1).transform.GetChild(1).transform.GetChild(0).transform.GetChild(4).transform.GetChild(0).transform.GetChild(1).transform.GetChild(2).gameObject;
        return rightHand;
    }

    private Node BuildTreeRoot()
    {
        print("build");
        return
            new DecoratorLoop(
                new Sequence( //Wandering behavior
                    new Sequence(this.ST_Approach(this.destination1)),
                    new Sequence(this.ST_Approach(this.destination2)),
                    new Sequence(this.ST_Approach(this.destination3)),
                    new Sequence(this.ST_Approach(this.destination4))
                     )
                );
        
    }
}